#include <xchange/base/Thread.h>
#include <xchange/base/ThreadPool.h>

using xchange::threadPool::Task;
using xchange::threadPool::ThreadPool;

#include <string>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

long getClock()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

struct msg
{
    double param1;
    double param2;
    double param3;
    double param4;
    double param5;
    char str1[20];
    char str2[20];
    char str3[20];
};

const int msgSize = sizeof(struct msg);

const int length = 20;

const int count = 1230;

const int workItemCount = 10000000;


struct bigPacket
{
    int msgCount;
    char buf[msgSize * count];
};

void* calcString(void *arg)
{
    struct bigPacket * m_bigPacket = static_cast<struct bigPacket *>(arg);
    if(m_bigPacket)
    {
        int size = m_bigPacket->msgCount;
        struct msg * tempMsg = new struct msg();
        for(int i = 0; i < size; ++i)
        {
            memcpy(tempMsg,m_bigPacket->buf + i * msgSize,msgSize);
        }
        delete(tempMsg);
    }

    return NULL;
}

void test_threadpool(int maxSize)
{
    ThreadPool pool(3, maxSize);
    pool.start();

    long m_time = getClock();
    double temp = 0;
    int count = 0;
    struct bigPacket * packet = new struct bigPacket();
    packet->msgCount = count;
    for(int i = 0; i < count; ++i){
        struct msg * ptr = new struct msg();
        ptr->param1 = temp++;
        memset(ptr->str1,'7',length);
        memcpy(packet->buf + i * msgSize, ptr, msgSize);
        delete ptr;
    }

    for (int i = 0; i < workItemCount; ++i)
    {
        int ret = 0;
        Task *task = new Task(calcString, packet, true);
        ret = pool.execute(task);
        if (ret != 0) {
            std::cout << "excute failed: " << ret << std::endl;
        }
    }
    std::cout << "WORK DONE -----> : " << getClock() - m_time << std::endl;

    while(1) {
        ThreadPool::checkResult();

        ThreadPool::Status s = pool.getStatus();
        if (s.busyThread == 0 && s.unhandledTask == 0) {
            // handle pengind signal
            ThreadPool::checkResult();
            break;
        } else {
            std::cout << "current busyThread and unhandledTask: " << s.busyThread << " " << s.unhandledTask << std::endl;
        }
    }

    delete packet;
}


int main()
{
    //test(0);
    //test(1);
    //test(5);
    //test(10);
    //test(50);

    test_threadpool(10000);

}
