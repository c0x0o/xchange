#include "base/Mutex.h"

using xchange::base::Mutex;

Mutex::Mutex() : isLocked_(false) {
    pthread_mutexattr_init(&attr_);
    pthread_mutex_init(&lock_, &attr_);
}

Mutex::~Mutex() {
    pthread_mutex_destroy(&lock_);
    pthread_mutexattr_destroy(&attr_);
}

int Mutex::lock() {
    int ret = pthread_mutex_lock(&lock_);

    if (ret == 0) {
        isLocked_ = true;
    }

    return ret;
}

bool Mutex::locked() {
    return isLocked_;
}

int Mutex::unlock() {
    int ret = pthread_mutex_unlock(&lock_);

    if (ret == 0) {
        isLocked_ = false;
    }

    return ret;
}
