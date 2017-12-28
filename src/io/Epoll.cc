#include "io/Epoll.h"
#include "io/Cache.h"
#include "io/Buffer.h"

#include <iostream>

using xchange::EventEmitter;
using xchange::io::EpollEvent;
using xchange::io::EPOLL_READ;
using xchange::io::EPOLL_WRITE;
using xchange::io::EPOLL_ERROR;
using xchange::io::Epoll;
using xchange::io::EpollContext;
using xchange::io::Cache;
using xchange::io::Buffer;

EpollContext::EpollContext(int watchfd, int evts, uint64_t cacheSize)
    : fd_(watchfd),
    events_(evts),
    readable_(false),
    writeable_(true),
    fatal_(false),
    readCache_(cacheSize),
    writeCache_(cacheSize)
{
}
EpollContext::~EpollContext() {}

#define CHUNK_SIZE (1024*8)

int EpollContext::readIntoCache_() {
    uint64_t finalRead = 0, tempSize = readCache_.maxSize() < CHUNK_SIZE ? CHUNK_SIZE:readCache_.maxSize();
    uint8_t *temp = new uint8_t[tempSize];

    if (!readable_) {
        delete []temp;
        return -1;
    }

    while (1) {
        uint64_t canRead = readCache_.usableSize() < CHUNK_SIZE ? readCache_.usableSize() : CHUNK_SIZE;
        int64_t nread = 0;

        memset(temp, 0, tempSize);

        if (canRead == 0) {
            break;
        }

        nread = ::read(fd_, temp, canRead);
        if (nread < 0) {
            if (errno == EINTR) {
                continue;
            }

            readable_ = false;

            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fatal_ = true;
            }

            break;
        }

        finalRead += nread;

        readCache_.write(Buffer(temp, nread));
    }

    delete []temp;
    return finalRead;
}

int EpollContext::writeFromCache_(bool needFlush) {
    uint64_t finalWrite = 0;

    if (!writeable_) return -1;

    while (1) {
        uint64_t canWrite = writeCache_.size() < CHUNK_SIZE ? writeCache_.size() : CHUNK_SIZE;
        int64_t nwrite = 0;

        if (canWrite == 0) {
            break;
        }

        if (canWrite < CHUNK_SIZE && !needFlush) {
            break;
        }

        Buffer buff = writeCache_.read(canWrite);
        nwrite = ::write(fd_, buff.expose(), buff.size());
        if (nwrite < 0) {
            if (errno == EINTR) {
                continue;
            }

            writeable_ = false;

            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fatal_ = true;
                return -1;
            }

            break;
        }

        finalWrite += nwrite;
    }

    return finalWrite;
}

#undef CHUNK_SIZE

Buffer EpollContext::read(uint64_t length) {
    int ret = 0;

    if (!readable_) return Buffer();

    readIntoCache_();

    // read everything
    if (length == 0) {
        Buffer result;

        do {
            ret = readIntoCache_();

            result += readCache_.read(readCache_.size());

        } while (readable_);

        return result;
    }

    // read specific size
    if (length <= readCache_.size()) {
        return readCache_.read(length);
    } else {
        Buffer result;

        do {
            ret = readIntoCache_();

            result += readCache_.read(length - result.size());

        } while (ret > 0 && result.size() < length);

        return result;
    }
}

int64_t EpollContext::write(const Buffer & content) {
    uint64_t nwrite = writeCache_.usableSize();;

    if (!writeable_) return -1;

    if (nwrite >= content.size()) {
        writeCache_.write(content);

        return (int64_t)content.size();
    }

    writeFromCache_();

    nwrite = 0;
    do {
        uint64_t usable = writeCache_.usableSize();
        uint64_t needWrite = content.size()-nwrite > usable ? usable : content.size()-nwrite;

        writeCache_.write(content.slice(nwrite, needWrite));

        if (writeCache_.usableSize() == 0) {
            writeFromCache_();
        }

        nwrite += needWrite;
    } while (writeable_ && nwrite < content.size());

    return (uint64_t)nwrite;
}

void EpollContext::flush() {
    writeFromCache_(true);
}

Epoll::Epoll(uint64_t cacheSize, int maxEvent)
    : cacheSize_(cacheSize),
    maxEvent_(maxEvent)
{
    epollfd_ = epoll_create(1);

    if (epollfd_ == -1) {
        throw std::runtime_error("epoll_create1 failed");
    }
}

Epoll::~Epoll() {
    ctx_.each(
        [](EpollContext *ctx) {
            ctx->flush();
            delete ctx;
        }
    );

    ::close(epollfd_);
}

int Epoll::setNonblocking_(int fd) {
    int old = fcntl(fd, F_GETFL);

    return fcntl(fd, F_SETFL, old|O_NONBLOCK);
}

int Epoll::watch(int fd, int e) {
    struct epoll_event ev;
    EpollContext *ctx = NULL;
    int ret;

    ret = setNonblocking_(fd);
    if (ret < 0) {
        return -1;
    }

    ev.events = EPOLLET;
    if (e & EPOLL_READ) {
        ev.events |= EPOLLIN;
    }
    if (e & EPOLL_ERROR) {
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
        ctx = new EpollContext(fd, e, cacheSize_);

        ev.data.ptr = ctx;

        ret = epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev);
        if (ret == 0) {
            ctx_.insert(fd, ctx);
        } else {
            delete ctx;
        }
    }

    return ret;
}

int Epoll::unwatch(int fd, int e) {
    if (e == 0) {
        // remove this fd
        int ret = epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, NULL);

        try {
            EpollContext * ctx = ctx_.find(fd);

            delete ctx;
        } catch (const std::exception &err) {}

        return ret;
    }

    // only remove some events
    struct epoll_event ev;
    EpollContext * ctx = NULL;
    int ret;

    try {
        ctx = ctx_.find(fd);

        e = ctx->event(ctx->event() & (ctx->event() ^ e));

        ev.events = EPOLLET;
        if (e & EPOLL_READ) {
            ev.events |= EPOLLIN;
        }
        if (e & EPOLL_ERROR) {
            ev.events |= EPOLLERR;
        }

        ev.data.ptr = ctx;

        ret = epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev);
    } catch (std::exception err) {
        return -1;
    }

    return ret;
}

void Epoll::tick() {
    // handle events which happened in last tick and were not fully processed:
    // 1. not all data in ReadCache are read
    for (uint64_t i = unhandled_.size(); i>0; i--) {
        EpollContext *ctx = unhandled_.front();
        unhandled_.pop_front();

        if (ctx->event() & EPOLL_READ && (ctx->readCache_.size() > 0 || ctx->readable_)) {
            emit(EPOLL_READ, ctx);

            if (ctx->readCache_.size() > 0 || ctx->readable_) {
                unhandled_.push_back(ctx);
            }
        }
    }
    // 2. flush all data remained in readCache
    ctx_.each(
        [](EpollContext *ctx) {
            ctx->flush();
        }
    );

    // epoll tick
    struct epoll_event events[maxEvent_];
    int eventNum = epoll_wait(epollfd_, events, maxEvent_, -1);

    // handle event happened in this tick
    for (int i = 0; i < eventNum; i++) {
        EpollContext *ctx = static_cast<EpollContext *>(events[i].data.ptr);

        if (ctx->fatal_) {
            // handle error event
            ctx->readable_ = false;
            ctx->writeable_ = false;
            emit(EPOLL_ERROR, ctx);

            ctx_.remove(ctx->fd());
            delete ctx;

            continue;
        }

        if (ctx->event() & EPOLL_READ && events[i].events & EPOLLIN) {
            // handle read
            ctx->readable_ = true;
            emit(EPOLL_READ, ctx);

            // still have data in cache
            if (ctx->readCache_.size() > 0 || ctx->readable_) {
                unhandled_.push_back(ctx);
            }
        }
        if (ctx->event() & EPOLL_WRITE && events[i].events & EPOLLOUT) {
            // handle write event, but you hardly need to do that
            // because in most cases, fd is always writeable
            ctx->writeable_ = true;
            emit(EPOLL_WRITE, ctx);
        }
    }
}
