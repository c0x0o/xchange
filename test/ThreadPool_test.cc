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

std::string getCountTime();
std::string getTime();
long getClock();

std::string getCountTime()
{
    time_t nowTime;
    time(&nowTime);

    nowTime = nowTime + 8 * 3600;

    char str[20];
    std::string ret;

    sprintf(str,
            "%ld",
            nowTime);
    ret = str;
    return ret;
}
std::string getTime()
{
    time_t nowTime;
    time(&nowTime);

    nowTime = nowTime + 8 * 3600;

    tm * t = gmtime(&nowTime);

    std::string ret;
    char str[100];

    sprintf(str,
            "%d-%02d-%02d %02d:%02d:%02d\n",
            t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday,
            t->tm_hour,
            t->tm_min,
            t->tm_sec);
    ret = str;
    return ret;
}

long getClock()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}


void print()
{
    printf("tid=%lu\n", xchange::thread::CurrentThread::getThreadInfo().tid());
}

void printString(const std::string& str)
{
    std::cout << str;
    usleep(100*1000);
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

const int workItemCount = 1000000;


struct bigPacket
{
    int msgCount;
    char * buf = new char[msgSize * count];
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
    struct bigPacket * packet = new struct bigPacket();
    packet->msgCount = count;
    for(int i = 0; i < count; ++i){
        struct msg * ptr = new struct msg();
        ptr->param1 = temp++;
        memset(ptr->str1,'7',length);
        memcpy(packet->buf + i * msgSize, ptr, msgSize);
    }

    for (int i = 0; i < workItemCount; ++i)
    {
        pool.execute(new Task(calcString, packet));
    }
    std::cout << "WORK DONE -----> : " << getClock() - m_time << std::endl;

    while(1) {
        ThreadPool::checkResult();
        if (pool.getStatus().busyThread == 0) {
            break;
        }
    }

    pool.terminate();
}


int main()
{
    //test(0);
    //test(1);
    //test(5);
    //test(10);
    //test(50);

    test_threadpool(1000);

}
