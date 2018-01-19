#include <xchange/io/EpollManager.h>
#include <xchange/io/Cache.h>
#include <xchange/io/Buffer.h>

#include <iostream>

using xchange::EventEmitter;
using xchange::io::IOEvent;
using xchange::io::IO_READ;
using xchange::io::IO_WRITE;
using xchange::io::IO_ERROR;
using xchange::io::EpollManager;
using xchange::io::IOContext;
using xchange::io::Cache;
using xchange::io::Buffer;

EpollManager::EpollManager(uint64_t cacheSize, int maxEvent)
    : cacheSize_(cacheSize),
    maxEvent_(maxEvent)
{
    epollfd_ = epoll_create(1);

    if (epollfd_ == -1) {
        throw std::runtime_error("epoll_create1 failed");
    }
}

EpollManager::~EpollManager() {
    ctx_.each(
        [](IOContext *ctx, int) {
            ctx->flush();
            delete ctx;
        }
    );

    ::close(epollfd_);
}

int EpollManager::setNonblocking_(int fd) {
    int old = fcntl(fd, F_GETFL);

    return fcntl(fd, F_SETFL, old|O_NONBLOCK);
}

IOContext* EpollManager::watch(int fd, int e) {
    struct epoll_event ev;
    IOContext *ctx = NULL;
    int ret;

    ret = setNonblocking_(fd);
    if (ret < 0) {
        return NULL;
    }

    ev.events = EPOLLET;
    if (e & IO_READ) {
        ev.events |= EPOLLIN;
    }
    if (e & IO_ERROR) {
        ev.events |= EPOLLERR;
    }

    try {
        ctx = ctx_.find(fd);

        ev.data.ptr = ctx;

        ret = epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev);
        if (ret == 0) {
            ctx->event(ctx->event() | e);
        }
    } catch (const std::exception & error) {
        ctx = new IOContext(fd, e, cacheSize_);

        ev.data.ptr = ctx;

        ret = epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev);
        if (ret == 0) {
            ctx_.insert(fd, ctx);
        } else {
            delete ctx;
        }
    }

    return ctx;
}

int EpollManager::unwatch(int fd, int e) {
    if (e == 0) {
        // remove this fd
        int ret = epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, NULL);

        try {
            IOContext * ctx = ctx_.find(fd);

            delete ctx;
        } catch (const std::exception &err) {}

        return ret;
    }

    // only remove some events
    struct epoll_event ev;
    IOContext * ctx = NULL;
    int ret;

    try {
        ctx = ctx_.find(fd);

        e = ctx->event(ctx->event() & (ctx->event() ^ e));

        ev.events = EPOLLET;
        if (e & IO_READ) {
            ev.events |= EPOLLIN;
        }
        if (e & IO_ERROR) {
            ev.events |= EPOLLERR;
        }

        ev.data.ptr = ctx;

        ret = epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev);
    } catch (std::exception err) {
        return -1;
    }

    return ret;
}

void EpollManager::tick() {
    // handle events which happened in last tick and were not fully processed:
    // 1. not all data in ReadCache are read
    for (uint64_t i = unhandled_.size(); i>0; i--) {
        IOContext *ctx = unhandled_.front();
        unhandled_.pop_front();

        if (ctx->event() & IO_READ && (ctx->readCache().size() > 0 || ctx->readable())) {
            emit(IO_READ, ctx);

            if (ctx->readCache().size() > 0 || ctx->readable()) {
                unhandled_.push_back(ctx);
            }
        }
    }
    // 2. flush all data remained in readCache
    ctx_.each(
        [](IOContext *ctx, int) {
            ctx->flush();
        }
    );

    // epoll tick
    struct epoll_event events[maxEvent_];
    int eventNum = epoll_wait(epollfd_, events, maxEvent_, -1);

    // handle event happened in this tick
    for (int i = 0; i < eventNum; i++) {
        IOContext *ctx = static_cast<IOContext *>(events[i].data.ptr);

        if (ctx->fatal()) {
            // handle error event
            ctx->readable(false);
            ctx->writeable(false);
            emit(IO_ERROR, ctx);

            ctx_.remove(ctx->fd());
            delete ctx;

            continue;
        }

        if (ctx->event() & IO_READ && events[i].events & EPOLLIN) {
            // handle read
            ctx->readable(true);
            emit(IO_READ, ctx);

            // still have data in cache
            if (ctx->readCache().size() > 0 || ctx->readable()) {
                unhandled_.push_back(ctx);
            }
        }
        if (ctx->event() & IO_WRITE && events[i].events & EPOLLOUT) {
            // handle write event, but you hardly need to do that
            // because in most cases, fd is always writeable
            ctx->writeable(true);
            emit(IO_WRITE, ctx);
        }
    }
}
