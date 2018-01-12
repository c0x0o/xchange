#include <xchange/base/Timer.h>

using xchange::base::TimerContext;
using xchange::base::Timer;
using xchange::base::TimerEvent;
using xchange::base::Timestamp;
using xchange::base::TimerLoop;
using xchange::algorithm::RedBlackTree;

void alarmInfoHandler(int sig, siginfo_t *info, void *) {
    Timer *timer = static_cast<Timer *>(info->si_ptr);

    if (sig != SIGALRM || timer == NULL) return;

    timer->timeout();
}

void *xchange::base::TimerLoop(void *arg) {
    TimerContext::TimerStorage & storage = static_cast<TimerContext *>(arg)->rbtree_;

    sigset_t set;
    union sigval data;
    siginfo_t siginfo;
    struct timespec zero = {0, 0};

    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);

    for (;;) {
        try {
            // trigger all timeouted timer
            while (storage.findSmallestPair().key < Timestamp::now()) {
                const TimerContext::TimerStorage::Node & sender = storage.findSmallestPair();

                data.sival_ptr = sender.value;
                pthread_sigqueue(sender.value->holder(), SIGALRM, data);

                storage.remove(sender.key);
            }

            // receive new timer, do polling
            while (1) {
                int ret = sigtimedwait(&set, &siginfo, &zero);

                if (ret < 0) {
                    if (errno == EAGAIN) {
                        break;
                    }
                    if (errno == EINTR) {
                        continue;
                    }

                    break;
                } else {
                    Timer *timer = static_cast<Timer *>(siginfo.si_ptr);
                    if (siginfo.si_signo == SIGUSR1) {
                        storage.insert(timer->triggerTime(), timer);
                    } else if (siginfo.si_signo == SIGUSR2) {
                        storage.remove(timer->triggerTime());
                    }
                }
            }

        } catch (const std::exception err) {
            //wait infinit
            sigwaitinfo(&set, &siginfo);
            Timer *timer = static_cast<Timer *>(siginfo.si_ptr);

            if (siginfo.si_signo == SIGUSR1) {
                storage.insert(timer->triggerTime(), timer);
            } else if (siginfo.si_signo == SIGUSR2) {
                storage.remove(timer->triggerTime());
            }
        }
    }

    return NULL;
}

TimerContext::TimerContext(): timerThread_(TimerLoop, "TimerThread") {
    sigset_t set;
    struct sigaction action;

    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = alarmInfoHandler;
    sigemptyset(&action.sa_mask);
    sigaction(SIGALRM, &action, NULL);

    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    timerThread_.run(this);
}

TimerContext::~TimerContext() {
}

// use SIGUSR1 to register timer
int TimerContext::registerTimer(Timer *timer) {
    union sigval data;

    data.sival_ptr = timer;
    return pthread_sigqueue(timerThread_.tid(), SIGUSR1, data);
}

// use SIGUSR1 to deregister timer
int TimerContext::deregisterTimer(Timer *timer) {
    union sigval data;

    data.sival_ptr = timer;
    return pthread_sigqueue(timerThread_.tid(), SIGUSR2, data);
}
