#ifndef _BASE_THREADPOOL_H_
#define _BASE_THREADPOOL_H_

#include <pthread.h>
#include <signal.h>

#include <atomic>
#include <vector>
#include <stdexcept>

#include "design/Noncopyable.h"
#include "base/Thread.h"
#include "base/EventEmitter.h"
#include "algorithm/LockFreeQueue.h"

namespace xchange {
    namespace threadPool {
        enum WorkerEvent {WORKER_INIT, WORKER_DESTROY};
        enum ThreadPoolEvent {POOL_INIT, POOL_DESTROY, TASK_START, TASK_COMPLETE, TASK_FAILED};
        class Task;
        class Worker;
        class ThreadPool;

        class Task {
            public:
                typedef void *(*fn)(void *);
                typedef uint64_t id;

                Task(fn, void *arg = NULL);
                Task(const Task & oldTask);
                ~Task();

                id getId() const {return taskId_;};
                bool isRunning() const {return status_ == 1;};
                bool completed() const {return status_ == 2;};
                void *getResult() const {return result_;};

                void run();
            private:
                const id taskId_;
                uint8_t status_; // 0: init 1: running 2:complete
                fn main_;
                void *arg_;
                void *result_;
                static std::atomic_uint64_t usableId_;
        };

        void * WorkerMain(void *);

        class Worker: public xchange::EventEmitter<WorkerEvent> {
            public:
                Worker(uint64_t queueSize, ThreadPool &parent);
                Worker(const Worker & oldWorker);
                ~Worker();

                bool isRunning() const {return running_;};
                bool isAlive() {return 0 == thread_.kill(0);};
                uint64_t taskQueueSize() const {return tasks_.size();}
                Task *currentTask() const {return currentTask_;};

                int addTask(Task *);
                void kill(int sig = SIGKILL);

                friend void * xchange::threadPool::WorkerMain(void *);
            private:
                bool running_;
                Task *currentTask_;
                ThreadPool &parent_;
                xchange::algorithm::LockFreeQueue<Task *> tasks_;
                xchange::thread::Thread thread_;
        };

        class ThreadPool : xchange::Noncopyable, public xchange::EventEmitter<ThreadPoolEvent> {
            public:
                typedef xchange::thread::Thread::routine Routine;

                ThreadPool(uint64_t numOfWorker = 10, uint64_t WorkerQueueSize = 64);
                ~ThreadPool();

                bool isRunning() const {return running_;};
                uint64_t size() const {return workers_.size();};

                Task::id execute(Routine, void *);
                void maintain();
                void destroy();
            private:
                bool running_;
                std::vector<Worker *> workers_;
        };

    }
}

#endif
