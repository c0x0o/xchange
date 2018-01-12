#ifndef XCHANGE_IO_EPOLL_H_
#define XCHANGE_IO_EPOLL_H_

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include <stdexcept>
#include <deque>

#include <xchange/base/EventEmitter.h>
#include <xchange/design/Noncopyable.h>
#include <xchange/io/Buffer.h>
#include <xchange/io/Cache.h>
#include <xchange/io/IOManager.h>
#include <xchange/algorithm/RedBlackTree.h>

namespace xchange {

namespace io {
    class EpollManager: public xchange::io::IOManager {
        public:
            EpollManager(uint64_t cacheSize = 16*1024, int maxEvent = 100);
            ~EpollManager();

            int watch(int fd, int e = xchange::io::IO_READ);
            int unwatch(int fd, int e = 0);
            void tick();
        private:
            uint64_t cacheSize_;
            int maxEvent_;
            int epollfd_;
            xchange::algorithm::RedBlackTree<int, xchange::io::IOContext *> ctx_;
            std::deque<xchange::io::IOContext *> unhandled_;

            int setNonblocking_(int fd);
    };

}

}

#endif
