#include <xchange/base/ThreadPool.h>

using xchange::thread::CurrentThread;
using xchange::threadPool::Task;
using xchange::threadPool::TaskEvent;
using xchange::threadPool::Worker;
using xchange::threadPool::workerMain;
using xchange::threadPool::ThreadPool;
using xchange::threadPool::ThreadPoolEvent;

std::atomic<uint64_t> Task::usableId_(0);

Task::Task(Task::routine fn, void *arg)
    : taskId_(usableId_.load(std::memory_order_relaxed)),
    status_(Task::INIT),
    main_(fn),
    arg_(arg),
    result_(NULL)
{}

void* Task::run() {
    if (status_ != INIT) return NULL;

    emit(TaskEvent::TASK_START, NULL);
    status_ = RUNNING;
    result_ = main_(arg_);
    status_ = COMPLETE;
    emit(TaskEvent::TASK_COMPLETE, result_);

    return result_;
}

void *xchange::threadPool::workerMain(void *arg) {
    Worker &worker = *static_cast<Worker *>(arg);

    sigset_t set;
    siginfo_t info;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR2);

    while (1) {
        if (worker.queueSize() == 0) {
            sigwaitinfo(&set, &info);
        }

        while (1) {
            try {
                Task* taskp = worker.tasks_.shift();

                if (bool(taskp)) {
                    union sigval val;

                    worker.busy_ = true;
                    taskp->run();
                    worker.busy_ = false;

                    val.sival_ptr = taskp;
                    pthread_sigqueue(CurrentThread::getThreadInfo().mainid(), SIGUSR2, val);
                }
            } catch (const std::exception &err) {
                break;
            }
        }
    }

    return NULL;
}

Worker::Worker(uint64_t queueSize)
    : busy_(false),
    maxSize_(queueSize),
    tasks_(queueSize),
    thread_(workerMain, "ThreadPoolWorker") {

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR2);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    thread_.detach();
    thread_.run(this);
}
Worker::~Worker() {
    thread_.kill();
}

bool Worker::alive() {
    return 0 == thread_.kill(0);
}

void Worker::kill() {
    thread_.kill();
}

void Worker::restart() {
    if (!alive()) {
        thread_.run(this);
    }
}

int Worker::addTask(Task *taskptr) {
    int ret = 0;

    taskptr->emit(TaskEvent::TASK_START, NULL);
    try {
        tasks_.push(taskptr);
    } catch(const std::exception &err) {
        ret = 1;
    }

    union sigval val;
    val.sival_ptr = NULL;
    pthread_sigqueue(thread_.tid(), SIGUSR2, val);

    return ret;
}

ThreadPool::ThreadPool(uint64_t numOfWorker, uint64_t WorkerQueueSize)
    : running_(false),
    numOfWorker_(numOfWorker),
    workerQueueSize_(WorkerQueueSize),
    workers_() {
}
ThreadPool::~ThreadPool() {
    terminate();
}

void ThreadPool::checkResult() {
    siginfo_t info;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR2);

    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;

    while (1) {
        int ret = sigtimedwait(&set, &info, &t);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }

        Task * taskp = static_cast<Task *>(info.si_ptr);

        if (taskp != NULL) {
            taskp->emit(TaskEvent::TASK_COMPLETE, taskp);
        }
    }
}

ThreadPool::Status ThreadPool::getStatus() const {
    uint64_t size = workers_.size();
    uint64_t aliveNum = 0;
    uint64_t busyNum = 0;
    uint64_t unhandledNum = 0;

    for (uint64_t i = 0; i < size; i++) {
        if (workers_[i]->alive()) {
            aliveNum++;

            if (workers_[i]->busy()) {
                busyNum++;
            }
        }

        unhandledNum += workers_[i]->queueSize();
    }

    return Status(size, aliveNum, busyNum, unhandledNum);
}

int ThreadPool::start() {
    for (uint64_t i = 0; i < numOfWorker_; i++) {
        workers_.push_back(new Worker(workerQueueSize_));
    }

    running_ = true;
    emit(ThreadPoolEvent::POOL_INIT, NULL);

    return 0;
}

int ThreadPool::execute(Task *ptr) {
    uint64_t min = workerQueueSize_, which = 0;

    if (ptr == NULL || !running_) {
        return -1;
    }

    for (uint64_t i = 0; i < workers_.size(); i++) {
        if (workers_[i]->queueSize() == 0 && !workers_[i]->busy()) {
            which = i;
            min = workers_[i]->queueSize();
            break;
        }

        if (workers_[i]->queueSize() < min) {
            which = i;
            min = workers_[i]->queueSize();
        }
    }

    if (min == workerQueueSize_) {
        return -1;
    }

    workers_[which]->addTask(ptr);

    return 0;
}

void ThreadPool::maintain() {
    for (uint64_t i = 0; i < workers_.size(); i++) {
        workers_[i]->restart();
    }
}

void ThreadPool::terminate() {
    for (uint64_t i = 0; i < workers_.size(); i++) {
        workers_[i]->kill();
    }

    checkResult();

    running_ = false;
    emit(ThreadPoolEvent::POOL_TERMINATE, NULL);
}
