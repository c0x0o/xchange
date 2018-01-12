#include <xchange/base/ThreadPool.h>

#include <unistd.h>

#include <iostream>
#include <string>

using std::cout;
using std::cin;
using std::endl;
using xchange::thread::currentThread;
using xchange::threadPool::ThreadPool;
using xchange::threadPool::ThreadPoolEvent;
using xchange::threadPool::POOL_INIT;
using xchange::threadPool::POOL_DESTROY;
using xchange::threadPool::TASK_COMPLETE;

void* threadAMain(void *arg) {
    std::string *str = static_cast<std::string *>(arg);

    while (1) {
        cout << currentThread.tid() << ": argument->" << *str << endl;
        sleep(1);
    }

    return str;
}

void outputResult(ThreadPoolEvent, void *str) {
    cout << "output: " << *static_cast<std::string *>(str) << endl;
}

void onDestroy(ThreadPoolEvent, void *) {
    cout << "Thread Pool Destroyed" << endl;
}

int main(void) {
    ThreadPool pool;
    std::string a = "AThread", b = "BThread", c = "CThread";

    pool.on(POOL_DESTROY, onDestroy);
    pool.on(TASK_COMPLETE, outputResult);

    pool.execute(threadAMain, &a);
    pool.execute(threadAMain, &b);
    pool.execute(threadAMain, &c);

    pause();

    return 0;
}
