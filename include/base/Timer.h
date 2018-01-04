#ifndef _BASE_TIMER_H_
#define _BASE_TIMER_H_

#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#include <stdexcept>
#include <string>
#include <utility>

#include "design/Noncopyable.h"
#include "design/Singleton.h"
#include "base/EventEmitter.h"
#include "base/Thread.h"
#include "base/Timestamp.h"
#include "base/Mutex.h"
#include "algorithm/RedBlackTree.h"

namespace xchange {

namespace base {
    enum TimerEvent {TIMER_TIMEOUT};
    class Timer;
    class TimerContext;

    void *TimerLoop(void *arg);

    class TimerContext {
        public:
            typedef xchange::algorithm::RedBlackTree<xchange::base::Timestamp, Timer*> TimerStorage;

            explicit TimerContext();
            ~TimerContext();

            int registerTimer(Timer *timer);
            int deregisterTimer(Timer *timer);

            friend void *TimerLoop(void *arg);
        private:
            TimerStorage rbtree_;
            xchange::thread::Thread timerThread_;
    };

    class Timer: xchange::Noncopyable, public xchange::EventEmitter<TimerEvent> {
        public:
            typedef void (*fn)(TimerEvent, void *);

            explicit Timer(xchange::base::Timestamp &ts, void *arg)
                :triggerTime_(ts),
                holder_(xchange::thread::currentThread.tid()),
                arg_(arg)
            {
            };
            ~Timer() {};

            pthread_t holder() const {return holder_;}
            const Timestamp &triggerTime() const {return triggerTime_;}
            void timeout() {EventEmitter::emit(TIMER_TIMEOUT, arg_);}
        private:
            xchange::base::Timestamp triggerTime_;
            pthread_t holder_;
            void *arg_;
    };
}

}

#endif
