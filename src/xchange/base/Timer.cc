#include <xchange/base/Timer.h>

using xchange::thread::Thread;
using xchange::thread::ThreadMessage;
using xchange::thread::CurrentThread;
using xchange::base::Timer;
using xchange::base::TimerEvent;
using xchange::base::Timestamp;
using xchange::base::TimerLoop;
using xchange::base::TimerStorage;
using xchange::base::TimerManager;
using xchange::base::TimerMessage;
using xchange::algorithm::RedBlackTree;

TimerStorage TimerManager::storage_;
Thread TimerManager::timerThread_(TimerLoop, "XChangeTimerThread");

void *xchange::base::TimerLoop(void *) {
    TimerStorage & storage = TimerManager::storage_;

    for (;;) {
        storage.raiseOutdatedTimer();

        while (1) {
            ThreadMessage::ptr msg = CurrentThread::receiveMessage(0);

            if (bool(msg)) {
                TimerMessage *body = static_cast<TimerMessage *>(msg->data);

                if (body->op == 0) {
                    // remove
                    storage.removeTimer(body->timer);
                } else {
                    // add
                    storage.addTimer(body->timer);
                }
            } else {
                break;
            }
        }
    }

    return NULL;
}

void TimerStorage::addTimer(Timer *timer) {
    StorageGuard guard(storage_);
    RawStorage &rbtree = *guard.getResource();

    rbtree.insert(timer->exceedTime(), timer);
}

void TimerStorage::removeTimer(Timer *timer) {
    StorageGuard guard(storage_);
    RawStorage &rbtree = *guard.getResource();

    rbtree.remove(timer->exceedTime());
}

void TimerStorage::raiseOutdatedTimer() {
    StorageGuard guard(storage_);
    RawStorage &rbtree = *guard.getResource();

    while (1) {
        try {
            const RawStorage::Node &node = rbtree.findSmallestPair();

            if (Timestamp::now() >= node.key) {
                union sigval val;
                val.sival_ptr = node.value;
                pthread_sigqueue(node.value->holder(), SIGALRM, val);

                rbtree.remove(node.key);
            } else {
                break;
            }
        } catch(const std::exception &err) {
            break;
        }
    }
}

int TimerManager::registerTimer(Timer *timer) {
    TimerMessage *add = new TimerAddMessage(timer);

    if (!timerThread_.running()) {
        timerThread_.run();
    }

    return timerThread_.sendMessage(add);
}

int TimerManager::deregisterTimer(Timer *timer) {
    TimerMessage *del = new TimerRemoveMessage(timer);

    if (!timerThread_.running()) {
        timerThread_.run();
    }

    return timerThread_.sendMessage(del);
}

void TimerManager::collectOutdatedTimer() {
    siginfo_t info;
    sigset_t set;
    struct timespec t;

    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    t.tv_sec = 0;
    t.tv_nsec = 0;

    while (1) {
        int ret = sigtimedwait(&set, &info, &t);

        if (ret < 0) {
            if (errno != EINTR) {
                break;
            }
            continue;
        }

        Timer * timer = static_cast<Timer *>(info.si_ptr);
        timer->trigger();
    }
}
