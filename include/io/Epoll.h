#ifndef _IO_EPOLL_H_
#define _IO_EPOLL_H_

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include <stdexcept>
#include <deque>

#include "base/EventEmitter.h"
#include "design/Noncopyable.h"
#include "io/Buffer.h"
#include "io/Cache.h"
#include "algorithm/RedBlackTree.h"

namespace xchange {

namespace io {
    enum EpollEvent {EPOLL_READ = 1, EPOLL_WRITE = 2, EPOLL_ERROR = 4, EPOLL_DISTROY = 100};
    class EpollContext;
    class Epoll;

    class EpollContext {
        public:
            EpollContext(int watchfd, int evts, uint64_t cacheSize);
            ~EpollContext();

            int fd() const {return fd_;}
            bool readable() const {return readable_;}
            bool writeable() const {return writeable_;}
            bool fatalError() const {return fatal_;}
            int event(void) const {return events_;}
            int event(int e) {return events_ = e;}
            Buffer read(uint64_t length = 0);
            int64_t write(const Buffer &buff);
            void flush();

            friend class Epoll;
        private:
            const int fd_;
            int events_;
            bool readable_;
            bool writeable_;
            bool fatal_;
            xchange::io::Cache readCache_;
            xchange::io::Cache writeCache_;

            int readIntoCache_();
            int writeFromCache_(bool needFlush = false);
    };

    class Epoll: xchange::Noncopyable, public xchange::EventEmitter<EpollEvent> {
        public:
            Epoll(uint64_t cacheSize = 16*1024, int maxEvent = 100);
            ~Epoll();

            int watch(int fd, int e = EPOLL_READ);
            int unwatch(int fd, int e = 0);
            void tick();
        private:
            uint64_t cacheSize_;
            int maxEvent_;
            int epollfd_;
            xchange::algorithm::RedBlackTree<int, EpollContext *> ctx_;
            std::deque<EpollContext *> unhandled_;

            int setNonblocking_(int fd);
    };

}

}

#endif
