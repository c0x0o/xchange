#include "base/Condition.h"

using xchange::Condition;
using xchange::Mutex;

Condition::Condition(Mutex &mutex) : lock_(mutex) {
    pthread_condattr_init(&attr_);
    pthread_cond_init(&cond_, &attr_);
}

Condition::~Condition() {
    pthread_condattr_destroy(&attr_);
    pthread_cond_destroy(&cond_);
}

int Condition::wait() {
    lock_.lock();

    int retval = pthread_cond_wait(&cond_, &lock_.lock_);

    return retval;
}

int Condition::waitFor(const double time) {
    struct timespec timer;
    static const int64_t nsPerSeconds = 1e9;
    int64_t nanoSeconds = static_cast<int64_t>(time * nsPerSeconds);
    int ret;

    clock_gettime(CLOCK_REALTIME, &timer);

    timer.tv_sec += static_cast<time_t>((timer.tv_nsec + nanoSeconds) / nsPerSeconds);
    timer.tv_nsec = static_cast<long>((timer.tv_nsec + nanoSeconds) % nsPerSeconds);

    ret = lock_.lock();
    if (!ret) {
        return ret;
    }

    ret = pthread_cond_timedwait(&cond_, &lock_.lock_, &timer);

    return ret;
}

int Condition::signal() {
    return pthread_cond_signal(&cond_);
}

int Condition::broadcast() {
    return pthread_cond_broadcast(&cond_);
}
