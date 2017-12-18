#include "base/Thread.h"

using xchange::thread::Thread;
typedef xchange::thread::Thread::routine routine;

xchange::thread::ThreadData & xchange::thread::currentThread = Singleton<ThreadData>::instance();

std::atomic_uint64_t Thread::total_(0);
Thread::Thread(const routine func, const std::string &name)
    : tid_(0),
    userFunc_(func),
    isRunning_(false),
    joinable_(true),
    threadName_(name),
    result_(NULL)
{
    total_++;
}

Thread::~Thread() {
    if (isRunning_) {
        int retval = pthread_cancel(tid_);

        if (retval != 0) {
            EventEmitter::emit(xchange::thread::ERROR, NULL);
        }
        EventEmitter::emit(xchange::thread::COMPLETE, NULL);
    }

    total_--;
}

int Thread::run(void *arg) {
    int retval;

    arg_ = arg;
    pthread_attr_init(&attr_);
    retval = pthread_create(&tid_, &attr_, xchange::thread::threadMain, this);

    if (retval != 0) {
        EventEmitter::emit(xchange::thread::ERROR, NULL);
    } else {
        isRunning_ = true;
    }

    return retval;
}

void *xchange::thread::threadMain(void *t) {
    Thread & thread = *(Thread *)t;
    void * result;

    xchange::thread::currentThread.tid(&thread.tid_);
    xchange::thread::currentThread.threadName(&thread.threadName_);

    thread.emit(xchange::thread::INIT, NULL);
    result = thread.userFunc_(thread.arg_);
    thread.emit(xchange::thread::COMPLETE, result);

    thread.isRunning_ = false;
    thread.result_ = result;

    return result;
}

int Thread::kill(int sig) {
    return pthread_kill(tid_, sig);
}

void* Thread::join() {
    if (joinable_) {
        pthread_join(tid_, &result_);
    }

    return result_;
}

void Thread::detach() {
    if (joinable_) {
        pthread_detach(tid_);
        joinable_ = false;
    }
}
