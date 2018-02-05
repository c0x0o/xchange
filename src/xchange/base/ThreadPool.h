#ifndef XCHANGE_BASE_THREADPOOL_H_
#define XCHANGE_BASE_THREADPOOL_H_

#include <pthread.h>
#include <signal.h>

#include <atomic>
#include <vector>
#include <functional>
#include <memory>
#include <stdexcept>

#include <xchange/design/Noncopyable.h>
#include <xchange/base/Thread.h>
#include <xchange/base/EventEmitter.h>
#include <xchange/algorithm/LockFreeQueue.h>

namespace xchange {
    namespace threadPool {
        enum TaskEvent {TASK_START, TASK_COMPLETE};
        enum ThreadPoolEvent {POOL_INIT, POOL_TERMINATE};

        class Task: public Noncopyable, public EventEmitter<TaskEvent> {
            public:
                typedef thread::Thread::routine routine;
                typedef uint64_t id;
                typedef enum {UNKNOWN = 0, INIT, RUNNING, COMPLETE} Status;
                typedef std::shared_ptr<Task> ptr;

                Task(routine, void *arg = NULL, bool recycle = false);
                ~Task() {};

                Task::id getId() const {return taskId_;}
                void* getResult() const {return result_;}
                Task::Status getStatus() const {return status_;}
                // automatically use 'delete' on task, related to 'recycle' in constructor
                // if you don't want to manage memory of Task yourself
                bool needRecycle() const {return recycle_;}

                void* run();
                void* operator()() {return run();};
            private:
                static std::atomic<uint64_t> usableId_;

                bool recycle_;
                const id taskId_;
                Status status_;
                routine main_;
                void *arg_;
                void *result_;
        };

        void *workerMain(void *);

        class Worker: public Noncopyable {
            public:
                Worker(uint64_t queueSize);
                ~Worker();

                bool busy() const {return busy_;}
                bool alive();
                bool queueSize() const {return tasks_.size();}

                void kill();
                void restart();
                int addTask(Task *taskp);

                friend void *workerMain(void *);
            private:
                bool busy_;
                uint64_t maxSize_;
                xchange::algorithm::LockFreeQueueSP<Task *> tasks_;
                xchange::thread::Thread thread_;
        };

        class ThreadPool : xchange::Noncopyable, public xchange::EventEmitter<ThreadPoolEvent> {
            public:
                typedef std::shared_ptr<ThreadPool> ptr;
                struct Status {
                    Status(uint64_t a, uint64_t b, uint64_t c, uint64_t d)
                        : totalThread(a),
                        aliveThread(b),
                        busyThread(c),
                        unhandledTask(d) {
                    }
                    ~Status() {}

                    const uint64_t totalThread;
                    const uint64_t aliveThread;
                    const uint64_t busyThread;
                    const uint64_t unhandledTask;
                };

                static void checkResult();

                ThreadPool(uint64_t numOfWorker = 8, uint64_t WorkerQueueSize = 64);
                ~ThreadPool();

                bool running() const {return running_;};
                uint64_t size() const {return workers_.size();};
                ThreadPool::Status getStatus() const;

                int start();
                int execute(Task *);
                void maintain();
                void terminate();
            private:
                bool running_;
                uint64_t numOfWorker_;
                uint64_t workerQueueSize_;
                std::vector<Worker *> workers_;
        };
    }
}

#endif
