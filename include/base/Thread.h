#ifndef _BASE_THREAD_H_
#define _BASE_THREAD_H_

#include <pthread.h>
#include <signal.h>

#include <string>
#include <map>
#include <atomic>

#include "design/Noncopyable.h"
#include "design/Singleton.h"
#include "base/EventEmitter.h"

#define getThreadKeyPtr(key, type) (*static_cast<type *>(pthread_getspecific(key)))
#define getThreadKey(key, type) (static_cast<type>(pthread_getspecific(key)))

namespace xchange {
    namespace thread {
        enum ThreadEvent {SUCCESS = 0, INIT = 1, COMPLETE = 2, ERROR = 3};

        void *threadMain(void *contextPtr);

        class Thread : xchange::Noncopyable, public xchange::EventEmitter<ThreadEvent> {
            public:
                typedef void * (*routine)(void *);
                typedef pthread_t id;

                explicit Thread(routine func, const std::string & name = std::string("Xchange_Thread"));
                ~Thread();

                bool joinable() const {return joinable_;};
                id tid() const {return tid_;};
                const std::string & threadName() const {return threadName_;}
                static uint64_t totalThreads() {return total_.load();}

                int run(void *arg = NULL);
                int kill(int);
                void* join();
                void detach();

                friend void *threadMain(void *contextPtr);
                friend struct ThreadData;
            private:
                pthread_t tid_;
                pthread_attr_t attr_;
                routine userFunc_;
                bool isRunning_;
                bool joinable_;
                std::string threadName_;
                void *result_;
                void *arg_;
                static std::atomic_uint64_t total_;
        };

        struct ThreadData {
            private:
                pthread_t mainid_;
                pthread_key_t tid_;
                pthread_key_t threadName_;
            public:
                ThreadData() {
                    mainid_ = pthread_self();
                    pthread_key_create(&tid_, NULL);
                    pthread_key_create(&threadName_, NULL);

                    pthread_setspecific(tid_, &mainid_);
                };
                ~ThreadData() {
                    pthread_key_delete(tid_);
                    pthread_key_delete(threadName_);
                };

                Thread::id mainid() const {return mainid_;};

                Thread::id tid() const {return getThreadKeyPtr(tid_, Thread::id);};
                void tid(Thread::id *id) {pthread_setspecific(tid_, id);};

                const std::string &threadName() const {return getThreadKeyPtr(threadName_, std::string);};
                void threadName(std::string *name) {pthread_setspecific(threadName_, name);};
        };

        extern ThreadData & currentThread;
    }
}

#endif
