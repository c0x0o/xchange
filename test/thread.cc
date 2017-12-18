#include "base/Thread.h"

#include <unistd.h>
#include <iostream>

using xchange::thread::Thread;
using xchange::thread::currentThread;
using xchange::thread::ThreadEvent;
using std::cout;
using std::endl;

void * threadFunc(void *arg) {
    long int i = 1;

    return (void *)i;
}

void onStart(xchange::thread::ThreadEvent e, void *arg) {
    cout << "[" << currentThread.threadName() << "] Thread " << currentThread.tid() << " starting " << endl;
}

void onComplete(xchange::thread::ThreadEvent e, void *arg) {
    cout << "main id: " << currentThread.mainid() << endl;
}

int main(void) {
    Thread a(threadFunc, "ThreadA"), b(threadFunc, "ThreadB");

    a.on(xchange::thread::INIT, onStart);
    a.on(xchange::thread::COMPLETE, onComplete);
    b.on(xchange::thread::INIT, onStart);
    b.on(xchange::thread::COMPLETE, onComplete);

    cout << "total threads : " << Thread::totalThreads() << endl;

    a.run();
    b.run();

    sleep(2);

    cout << "resultA:" << (long int)a.join() << endl;
    cout << "resultB:" << (long int)b.join() << endl;

    return 0;
}
