#ifndef XCHANGE_BASE_MUTEX_H_
#define XCHANGE_BASE_MUTEX_H_

#include <pthread.h>
#include <xchange/design/Noncopyable.h>

namespace xchange {

namespace base {
    class Mutex: xchange::Noncopyable {
        public:
            explicit Mutex();
            ~Mutex();
            int lock();
            bool locked();
            int unlock();

            friend class Condition;
        private:
            bool isLocked_;
            pthread_mutex_t lock_;
            pthread_mutexattr_t attr_;
    };

    class MutexLockGuard: xchange::Noncopyable {
        public:
            explicit MutexLockGuard(Mutex &lock): mutex_(lock) {
                mutex_.lock();
            }
            ~MutexLockGuard() {
                mutex_.unlock();
            }
        private:
            Mutex &mutex_;
    };

    template<typename T>
    class LockedResource: xchange::Noncopyable {
        public:
            explicit LockedResource(T *target): resource_(target), lock_() {}
            ~LockedResource() {
                if (lock_.locked()) {
                    lock_.unlock();
                }

                delete resource_;
            }

            T* getResource() {
                lock_.lock();

                return resource_;
            }

            void release() {
                if (lock_.locked()) {
                    lock_.unlock();
                }
            }
        private:
            T *resource_;
            Mutex lock_;
    };

    template<typename T>
    class LockedResourceGuard: xchange::Noncopyable {
        public:
            explicit LockedResourceGuard(LockedResource<T> &temp): resource_(temp.getResource()) {}
            ~LockedResourceGuard() {resource_->release();}

            T &getResource() {
                return *resource_;
            }
        private:
            T *resource_;
    };
}

}

#endif
