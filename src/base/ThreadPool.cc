#include "base/ThreadPool.h"

using xchange::threadPool::Task;
using xchange::threadPool::Worker;
using xchange::threadPool::WorkerMain;
using xchange::threadPool::WorkerEvent;
using xchange::threadPool::ThreadPool;
using xchange::threadPool::ThreadPoolEvent;

Task::Task(Task::fn handler, void *arg)
    : taskId_(usableId_.load(std::memory_order_relaxed)),
    status_(0),
    main_(handler),
    arg_(arg),
    result_(NULL)
{
    usableId_++;
}
std::atomic_uint64_t Task::usableId_(1);

Task::Task(const Task & oldTask)
    : taskId_(usableId_.load(std::memory_order_relaxed)),
    status_(0),
    main_(oldTask.main_),
    arg_(oldTask.arg_),
    result_(NULL)
{
    usableId_++;
}

Task::~Task() {}

void Task::run() {
    status_++;
    result_ = main_(arg_);
    status_++;
}

Worker::Worker(uint32_t queueSize, ThreadPool & parent)
    : running_(false),
    currentTask_(NULL),
    parent_(parent),
    tasks_(queueSize),
    thread_(WorkerMain)
{
    thread_.run(this);
    EventEmitter::on(WORKER_INIT, NULL);
}

Worker::Worker(const Worker & oldWorker)
    : running_(false),
    currentTask_(NULL),
    parent_(oldWorker.parent_),
    tasks_(oldWorker.tasks_),
    thread_(WorkerMain)
{
    thread_.run(this);
    EventEmitter::on(WORKER_INIT, NULL);
}

Worker::~Worker() {
    Task *task;

    if (isAlive()) {
        running_ = false;
        currentTask_ = NULL;
        kill();
    }

    if (thread_.joinable()) {
        thread_.join();
    }

    while (1) {
        try {
            task = tasks_.shift();
            delete task;
        } catch (std::out_of_range e) {
            break;
        }
    }

    EventEmitter::on(WORKER_DESTROY, NULL);
}

int Worker::addTask(Task * task) {
    if (!isAlive()) return 1;

    try {
        tasks_.push(task);
        thread_.kill(SIGUSR1);
    } catch (std::overflow_error e) {
        return 1;
    }

    return 0;
}

void Worker::kill(int sig) {
    thread_.kill(sig);
}

void *xchange::threadPool::WorkerMain(void *arg) {
    Worker *context = static_cast<Worker *>(arg);
    sigset_t set;
    int sig;

    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGKILL);

    while (1) {
        Task *task = NULL;
        try {
            task = context->tasks_.shift();
        } catch (std::out_of_range e) {
            sigwait(&set, &sig);

            if (sig == SIGKILL) break;

            continue;
        }

        context->currentTask_ = task;
        context->running_ = true;
        context->parent_.emit(xchange::threadPool::TASK_START, task);
        task->run();
        context->running_ = false;
        context->parent_.emit(xchange::threadPool::TASK_COMPLETE, task);
    }

    return NULL;
}

ThreadPool::ThreadPool(uint32_t numOfWorker, uint32_t workerQueueSize)
    : running_(true)
{
    sigset_t sigset;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigaddset(&sigset, SIGKILL);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    for (uint32_t i = 0; i < numOfWorker; i++) {
        Worker * worker = new Worker(workerQueueSize, *this);

        workers_.push_back(worker);
    }

    EventEmitter::emit(POOL_INIT, NULL);
}

ThreadPool::~ThreadPool() {
    destroy();
}

Task::id ThreadPool::execute(ThreadPool::Routine func, void *arg) {
    Task *task = new Task(func, arg);
    std::vector<Worker *>::iterator nextWorker;

    // balance load
    uint32_t min = 4294967295;
    for (std::vector<Worker *>::iterator i = workers_.begin(); i != workers_.end(); i++) {
        Worker & worker = **i;
        uint32_t queueSize = worker.taskQueueSize();

        if (queueSize < min || (queueSize == min && !worker.isRunning())) {
            nextWorker = i;
            min = queueSize;
        }
    };

    // add task failed
    int retry = 0;
    while ((*nextWorker)->addTask(task)) {
        if (retry < 3) {
            // try to fix it
            maintain();
            retry++;
        } else {
            // return an error
            return 0;
        }
    }

    return task->getId();
}

void ThreadPool::maintain() {
    for (std::vector<Worker *>::iterator i = workers_.begin(); i < workers_.end();) {
        Worker * worker = *i;

        if (!worker->isAlive()) {
            Worker *newOne = new Worker(*worker);

            delete worker;

            i = workers_.erase(i);
            workers_.push_back(newOne);
        } else {
            i++;
        }
    }
}

void ThreadPool::destroy() {
    running_ = false;
    for (std::vector<Worker *>::iterator i = workers_.begin(); i < workers_.end(); i++) {
        delete *i;
    }

    EventEmitter::emit(POOL_DESTROY, NULL);
}
