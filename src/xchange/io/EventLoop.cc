#include <xchange/io/EventLoop.h>

using xchange::io::EventLoop;
using xchange::io::channel::Channel;
using xchange::io::poller::Poller;
using xchange::threadPool::ThreadPool;
using xchange::base::TimerManager;

EventLoop::EventLoop(ThreadPool::ptr &pool, Poller::ptr &poller)
    : running_(false),
    pendingQueue_(2048),
    onceQueue_(2048),
    pool_(pool),
    poller_(poller)
{
    stableQueue_.reserve(128);
}

EventLoop::~EventLoop() {
    channels_.each([](Channel *channel, int) {
                channel->close();
                delete channel;
            });
}

int EventLoop::addChannel(Channel *channel) {
    int ret = poller_->bind(channel);

    if (ret > 0) {
        ret = channels_.insert(channel->fd(), channel);
    }

    return ret;
}

int EventLoop::removeChannel(Channel *channel) {
    int ret = poller_->unbind(channel);

    ret = channels_.remove(channel->fd());

    return ret;
}

int EventLoop::updateChannelEvent(Channel &channel) {
    int ret = poller_->updateChannelEvent(channel);

    return ret;
}

void EventLoop::runInLoop(const LoopItem &item) {
    try {
        onceQueue_.push(item);
    } catch (const std::exception &error) {
    }
}

void EventLoop::runInEveryTick(const LoopItem &item) {
    try {
        pendingQueue_.push(item);
    } catch (const std::exception &error) {
    }
}

int EventLoop::loop() {
    if (running_.load(std::memory_order_relaxed)) return 0;

    if (!pool_->running()) {
        pool_->start();
    }

    running_.store(true, std::memory_order_relaxed);

    while (running_.load(std::memory_order_release)) {
        // execute timer callback
        TimerManager::collectOutdatedTimer();

        poller_->poll(0);

        // collect LoopItem in pendingQueue_
        for (uint64_t currentSize = onceQueue_.size();
                currentSize > 0;
                currentSize--) {
            stableQueue_.push_back(pendingQueue_.shift());
        }

        for (uint64_t currentSize = onceQueue_.size();
                currentSize > 0;
                currentSize--) {
            (onceQueue_.shift())();
        }

        // execute stable LoopItem
        for (uint64_t i = 0; i < stableQueue_.size(); i++) {
            stableQueue_[i]();
        }

        ThreadPool::checkResult();
    }

    return 0;
}

void EventLoop::stop() {
    running_.store(false, std::memory_order_relaxed);
}
