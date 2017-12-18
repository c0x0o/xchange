#include "base/LockFreeQueue.h"

#include<unistd.h>

#include <cmath>
#include <iostream>
#include <vector>

#include "base/Thread.h"

using xchange::LockFreeQueue;
using xchange::thread::Thread;
using xchange::thread::currentThread;

LockFreeQueue<int> q(10000);
std::atomic_uint64_t i;

void * pushData(void *arg) {
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

void * popData(void *arg) {
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

void onCom(xchange::thread::ThreadEvent e, void *arg) {
    std::cout << "Terminated " << xchange::thread::currentThread.threadName() << std::endl;
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
