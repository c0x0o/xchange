#ifndef _BASE_MUTEX_H_
#define _BASE_MUTEX_H_

#include <pthread.h>
#include "design/Noncopyable.h"

namespace xchange {

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

}

#endif
