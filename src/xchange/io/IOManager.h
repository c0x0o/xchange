#ifndef XCHANGE_IO_MANAGER_H_
#define XCHANGE_IO_MANAGER_H_

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
#include <xchange/algorithm/RedBlackTree.h>

namespace xchange {

namespace io {
    enum IOEvent {IO_READ = 1, IO_WRITE = 2, IO_ERROR = 4, IO_DISTROY = 100};
    class IOContext;
    class IOManager;

    class IOContext {
        public:
            IOContext(int watchfd, int evts, uint64_t cacheSize);
            ~IOContext();

            int fd() const {return fd_;}

            bool readable() const {return readable_;}
            void readable(bool change) {readable_ = change;}

            bool writeable() const {return writeable_;}
            void writeable(bool change) {writeable_ = change;}

            bool fatal() const {return fatal_;}
            void fatal(bool change) {fatal_ = change;}

            int event() const {return events_;}
            int event(int e) {return events_ = e;}

            const xchange::io::Cache & readCache() const {return readCache_;}
            const xchange::io::Cache & writeCache() const {return writeCache_;}

            Buffer read(uint64_t length = 0);
            int64_t write(const Buffer &buff);
            void flush();
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

    class IOManager: xchange::Noncopyable, public xchange::EventEmitter<IOEvent> {
        public:
            virtual int watch(int fd, int e) = 0;
            virtual int unwatch(int fd, int e) = 0;
            virtual void tick() = 0;
    };

}

}

#endif
