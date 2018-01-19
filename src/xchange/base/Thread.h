#ifndef XCHANGE_BASE_THREAD_H_
#define XCHANGE_BASE_THREAD_H_

#include <pthread.h>
#include <signal.h>

#include <string>
#include <map>
#include <atomic>
#include <memory>
#include <functional>

#include <xchange/design/Noncopyable.h>
#include <xchange/design/Singleton.h>
#include <xchange/base/EventEmitter.h>

namespace xchange {
    namespace thread {
        enum ThreadEvent {SUCCESS = 0, INIT = 1, COMPLETE = 2, ERROR = 3};

        void *threadMain(void *);

        class Thread : public xchange::Noncopyable, public xchange::EventEmitter<ThreadEvent> {
            public:
                typedef std::function<void *(void *)> routine;
                typedef pthread_t id;

                explicit Thread(const routine func, const std::string & name = std::string("Xchange_Thread"));
                ~Thread();

                static uint64_t total() {return total_.load();}

                bool joinable() const {return joinable_;};
                bool running() const {return running_;};
                id tid() const {return tid_;};
                const std::string threadName() const {return threadName_;}
                routine getUserFunc() const {return userFunc_;}

                void run(void *arg = NULL);
                int kill(int sig, void *arg = NULL);
                int kill();
                int sendMessage(void *msg);
                void* join();
                void detach();

                friend void *threadMain(void *);
            private:
                static std::atomic<uint64_t> total_;

                pthread_t tid_;
                pthread_attr_t attr_;
                routine userFunc_;
                bool running_;
                bool joinable_;
                std::string threadName_;
                void *result_;
                void *arg_;
        };

        struct ThreadMessage {
            typedef std::shared_ptr<thread::ThreadMessage> ptr;

            ThreadMessage(Thread::id s, void *d):sender(s), data(d){};
            ~ThreadMessage(){};
            const Thread::id sender;
            void *data;
        };

        class ThreadData {
            public:
                ThreadData();
                ~ThreadData();

                Thread::id mainid() const {return mainid_;};

                Thread::id tid() const;
                void tid(Thread::id id) const;

                const std::string threadName() const;
                void threadName(const std::string &name);

            private:
                pthread_t mainid_;
                pthread_key_t tid_;
                pthread_key_t threadName_;
        };

        class CurrentThread: public xchange::Noncopyable {
            public:
                static ThreadData &getThreadInfo();
                static bool isCurrentThread(Thread &t);
                static bool isCurrentThread(Thread::id id);
                static bool isMainThread(Thread &t);
                static bool isMainThread(Thread::id id);
                static int sendMessage(Thread::id target, void *msg);
                static ThreadMessage::ptr receiveMessage(double timeout = -1);
            private:
                CurrentThread();

                static ThreadData data_;
        };
    }
}

#endif
