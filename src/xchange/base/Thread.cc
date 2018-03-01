#include <xchange/base/Thread.h>

using xchange::thread::Thread;
using xchange::thread::ThreadData;
using xchange::thread::CurrentThread;
using xchange::thread::threadMain;
using xchange::thread::ThreadMessage;
typedef xchange::thread::Thread::routine routine;

std::atomic<uint64_t> Thread::total_(0);

Thread::Thread(const routine func, const std::string &name)
    : tid_(0),
    userFunc_(func),
    running_(false),
    joinable_(true),
    threadName_(name),
    result_(NULL)
{
    total_++;
}

Thread::~Thread() {
    if (running_) {
        int retval = pthread_cancel(tid_);

        if (retval != 0) {
            emit(xchange::thread::ERROR, NULL);
        }
        emit(xchange::thread::COMPLETE, NULL);
    }

    total_--;
}

void Thread::run(void *arg) {
    int retval;

    if (running_ == true) {
        return;
    }

    arg_ = arg;
    pthread_attr_init(&attr_);
    if (joinable_ == false) {
        pthread_attr_setdetachstate(&attr_, PTHREAD_CREATE_DETACHED);
    }
    retval = pthread_create(&tid_, &attr_, xchange::thread::threadMain, this);

    if (retval != 0) {
        emit(xchange::thread::ERROR, NULL);
    } else {
        running_ = true;
    }
}

void *xchange::thread::threadMain(void *t) {
    Thread & thread = *(Thread *)t;
    void * result;

    CurrentThread::getThreadInfo().tid(thread.tid());
    CurrentThread::getThreadInfo().threadName(thread.threadName());

    thread.emit(xchange::thread::INIT, NULL);
    result = thread.userFunc_(thread.arg_);
    thread.emit(xchange::thread::COMPLETE, result);

    thread.running_ = false;
    thread.result_ = result;

    return result;
}

int Thread::kill(int sig, void *arg) {
    union sigval val;

    val.sival_ptr = arg;
    return pthread_sigqueue(tid_, sig, val);
}

int Thread::kill() {
    running_ = false;
    return pthread_cancel(tid_);
}

int Thread::sendMessage(void *arg) {
    return kill(SIGTRDMSG, new ThreadMessage(tid_, arg));
}

void* Thread::join() {
    if (joinable_) {
        pthread_join(tid_, &result_);
    }

    return result_;
}

void Thread::detach() {
    if (joinable_) {
        if (running_) {
            pthread_detach(tid_);
        }
        joinable_ = false;
    }
}

ThreadData::ThreadData() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGTRDMSG);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    mainid_ = pthread_self();

    pthread_key_create(&tid_, [](void *arg) {
            delete static_cast<Thread::id *>(arg);
            }
        );
    pthread_key_create(&threadName_, [](void *arg) {
            delete static_cast<std::string *>(arg);
            }
        );

    pthread_setspecific(tid_, new Thread::id(mainid_));
}

ThreadData::~ThreadData() {
    pthread_key_delete(tid_);
    pthread_key_delete(threadName_);
}

Thread::id ThreadData::tid() const {
    return *static_cast<Thread::id *>(pthread_getspecific(tid_));
}
void ThreadData::tid(Thread::id tid) const {
    pthread_setspecific(tid_, new Thread::id(tid));
}

const std::string ThreadData::threadName() const {
    return *static_cast<std::string *>(pthread_getspecific(threadName_));
}
void ThreadData::threadName(const std::string &threadName) {
    pthread_setspecific(threadName_, new std::string(threadName));
}

ThreadData CurrentThread::data_;

ThreadData &CurrentThread::getThreadInfo() {
    return data_;
}

bool CurrentThread::isCurrentThread(Thread &t) {
    if (t.tid() == data_.tid()) return true;

    return false;
}

bool CurrentThread::isCurrentThread(Thread::id id) {
    if (id == data_.tid()) return true;

    return false;
}

bool CurrentThread::isMainThread(Thread &t) {
    if (t.tid() == data_.mainid()) return true;

    return false;
}

bool CurrentThread::isMainThread(Thread::id id) {
    if (id == data_.mainid()) return true;

    return false;
}

int CurrentThread::sendMessage(Thread::id target, void *msg) {
    union sigval val;

    val.sival_ptr = msg;
    return pthread_sigqueue(target, SIGTRDMSG, val);
}

ThreadMessage::ptr CurrentThread::receiveMessage(double timeout) {
    sigset_t set;
    siginfo_t info;
    sigemptyset(&set);
    sigaddset(&set, SIGTRDMSG);

    while (1) {
        int ret;
        if (timeout < 0) {
            ret = sigwaitinfo(&set, &info);
        } else {
            struct timespec time;

            uint64_t ns = timeout * 1e9;
            time.tv_sec = static_cast<uint64_t>(ns) / static_cast<uint64_t>(1e9);
            time.tv_nsec = static_cast<uint64_t>(ns) % static_cast<uint64_t>(1e9);

            ret = sigtimedwait(&set, &info, &time);
        }

        if (ret > 0) {
            return ThreadMessage::ptr(static_cast<ThreadMessage *>(info.si_ptr));
        } else {
            if (errno != EINTR) {
                break;
            }
        }
    }

    return ThreadMessage::ptr();
}
