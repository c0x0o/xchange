#ifndef _BASE_CONDITION_H_
#define _BASE_CONDITION_H_

#include <pthread.h>
#include <time.h>

#include "design/Noncopyable.h"
#include "base/Mutex.h"

namespace xchange {

namespace base {
    class Condition : xchange::Noncopyable {
        public:
            Condition(Mutex &lock);
            ~Condition();
            int wait();
            int waitFor(const double time);
            int signal();
            int broadcast();
        private:
            pthread_cond_t cond_;
            pthread_condattr_t attr_;
            xchange::base::Mutex & lock_;
    };
}

}

#endif
