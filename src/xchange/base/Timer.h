#ifndef XCHANGE_BASE_TIMER_H_
#define XCHANGE_BASE_TIMER_H_

#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#include <stdexcept>
#include <string>
#include <utility>
#include <functional>

#include <xchange/design/Noncopyable.h>
#include <xchange/base/EventEmitter.h>
#include <xchange/base/Thread.h>
#include <xchange/base/Timestamp.h>
#include <xchange/base/Mutex.h>
#include <xchange/algorithm/RedBlackTree.h>

namespace xchange {

namespace base {
    enum TimerEvent {TIMER_TIMEOUT};

    void *TimerLoop(void *arg);

    class Timer: public EventEmitter<TimerEvent> {
        public:
            explicit Timer(const Timestamp &stamp, void *arg = NULL)
                : outdated_(false),
                time_(stamp),
                arg_(arg),
                holder_(xchange::thread::CurrentThread::getThreadInfo().tid()) {
            }
            ~Timer() {};

            bool outdated() {return outdated_;}
            Timestamp exceedTime() const {return time_;}
            xchange::thread::Thread::id holder() const {return holder_;}

            void trigger() {emit(TIMER_TIMEOUT, arg_);}
        private:
            bool outdated_;
            Timestamp time_;
            void *arg_;
            xchange::thread::Thread::id holder_;
    };

    class TimerStorage: public xchange::Noncopyable {
        public:
            typedef xchange::algorithm::RedBlackTree<Timestamp, Timer *> RawStorage;
            typedef xchange::base::LockedResource<RawStorage> Storage;
            typedef xchange::base::LockedResourceGuard<RawStorage> StorageGuard;

            TimerStorage(): storage_(new RawStorage()) {
                sigset_t set;
                sigemptyset(&set);
                sigaddset(&set, SIGALRM);

                pthread_sigmask(SIG_BLOCK, &set, NULL);
            };
            ~TimerStorage() {};

            void addTimer(Timer *);
            void removeTimer(Timer *);
            void raiseOutdatedTimer();
        private:
            Storage storage_;
    };

    class TimerManager: public xchange::Noncopyable {
        public:
            static int registerTimer(Timer *);
            static int deregisterTimer(Timer *);
            static void collectOutdatedTimer();

            friend void *TimerLoop(void *);
        private:
            TimerManager();

            static TimerStorage storage_;
            static xchange::thread::Thread timerThread_;
    };

    struct TimerMessage {
        TimerMessage(bool o, Timer *t): op(o), timer(t) {}
        ~TimerMessage() {}

        bool op;
        Timer *timer;
    };

#define TimerRemoveMessage(x) TimerMessage(0, (x))
#define TimerAddMessage(x) TimerMessage(1, (x))
}
}

#endif
