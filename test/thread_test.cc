#include <xchange/base/Thread.h>

#include <unistd.h>
#include <iostream>

using xchange::thread::Thread;
using xchange::thread::CurrentThread;
using xchange::thread::ThreadEvent;
using xchange::thread::ThreadMessage;
using std::cout;
using std::endl;

void * threadFunc(void *) {
    long int i = 0;

    while (1) {
        // shared_ptr
        ThreadMessage::ptr msg = CurrentThread::receiveMessage();

        if (msg == NULL) {
            continue;
        } else {
            i++;
            cout << CurrentThread::getThreadInfo().threadName() << " received a message: "
                <<*(std::string *)(msg->data) << " from " << msg->sender << endl;
        }

        if (CurrentThread::getThreadInfo().threadName() == "ThreadA" && i==2) break;
        if (CurrentThread::getThreadInfo().threadName() == "ThreadB" && i==1) break;
    }

    return (void *)i;
}

void onStart(xchange::thread::ThreadEvent, void *) {
    cout << "[" << CurrentThread::getThreadInfo().threadName() << "] Thread " << CurrentThread::getThreadInfo().tid() << " starting " << endl;
}

void onComplete(xchange::thread::ThreadEvent, void *) {
    cout << CurrentThread::getThreadInfo().threadName() << " terminated" << endl;
}

int main(void) {
    Thread a(threadFunc, "ThreadA"), b(threadFunc, "ThreadB");

    a.on(xchange::thread::INIT, onStart);
    a.on(xchange::thread::COMPLETE, onComplete);
    b.on(xchange::thread::INIT, onStart);
    b.on(xchange::thread::COMPLETE, onComplete);

    cout << "total threads : " << Thread::total() << endl;
    cout << "main thread id: " << CurrentThread::getThreadInfo().mainid() << endl;

    a.run();
    b.run();

    sleep(2);
    a.sendMessage(new std::string("a message incoming!"));
    sleep(2);
    b.sendMessage(new std::string("a message incoming!"));
    sleep(2);
    a.sendMessage(new std::string("a message incoming!"));
    sleep(2);

    cout << "threadA received " << (long int)a.join() << " messages" << endl;
    cout << "threadB received " << (long int)b.join() << " meesages" << endl;

    return 0;
}
