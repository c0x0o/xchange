#include <xchange/algorithm/LockFreeQueue.h>

#include <unistd.h>

#include <cmath>
#include <iostream>
#include <vector>

#include <xchange/base/Thread.h>

using xchange::algorithm::LockFreeQueue;
using xchange::algorithm::LockFreeQueueSP;
using xchange::thread::Thread;
using xchange::thread::CurrentThread;

LockFreeQueue<int> q(10000);
std::atomic<uint64_t> i;

void * pushData(void *) {
    while (1) {
        if (i.load() > 10000000) {
            break;
        }

        try {
            q.push(i);
            i++;
        } catch (std::overflow_error e) {
            // std::cout << "push wait" << std::endl;
        }
    }

    return nullptr;
}

void * popData(void *) {
    uint64_t i = 0;

    while (1) {
        int result = 0;

        if (i > 10000000) {
            break;
        }

        try {
            result = q.shift();
            // std::cout << result << std::endl;
            i++;
        } catch (std::out_of_range e) {
            // std::cout << "shift wait" << std::endl;
        }
    }

    return nullptr;
}

void onCom(xchange::thread::ThreadEvent, void *) {
    std::cout << "Terminated " << xchange::thread::CurrentThread::getThreadInfo().threadName() << std::endl;
}

int main(void) {
    Thread a(pushData, "ThreadA"), b(popData, "ThreadB");
    // Thread c(pushData, "ThreadC");

    i.store(0);

    a.run();
    b.run();
    // c.run();

    a.join();
    b.join();

    return 0;
}
