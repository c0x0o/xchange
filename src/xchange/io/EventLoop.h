#ifndef XCHANGE_IO_EVENTLOOP_H_
#define XCHANGE_IO_EVENTLOOP_H_

#include <vector>
#include <functional>
#include <atomic>

#include <xchange/algorithm/LockFreeQueue.h>
#include <xchange/algorithm/RedBlackTree.h>
#include <xchange/io/Channel.h>
#include <xchange/io/Poller.h>
#include <xchange/base/ThreadPool.h>
#include <xchange/base/Timer.h>

namespace xchange {
namespace io {
    class EventLoop {
        public:
            typedef std::function<void()> LoopItem;
            typedef xchange::algorithm::RedBlackTree<int, channel::Channel *> ChannelMap;
            typedef xchange::algorithm::LockFreeQueue<LoopItem> LoopItemQueue;

            EventLoop(xchange::threadPool::ThreadPool::ptr &pool,
                    xchange::io::poller::Poller::ptr &poller);
            ~EventLoop();

            bool running() const {return running_.load(std::memory_order_relaxed);}

            int addChannel(channel::Channel *channel);
            int removeChannel(channel::Channel *channel);
            int updateChannelEvent(channel::Channel &channel);

            void runInLoop(const LoopItem &);
            void runInEveryTick(const LoopItem &);

            int loop();

            void stop();
        private:
            std::atomic<bool> running_;

            ChannelMap channels_;
            std::vector<LoopItem> stableQueue_;
            LoopItemQueue pendingQueue_, onceQueue_;
            xchange::threadPool::ThreadPool::ptr pool_;
            xchange::io::poller::Poller::ptr poller_;
    };
}
}

#endif
