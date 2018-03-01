#ifndef XCHANGE_IO_CHANNEL_FSCHANNEL_H_
#define XCHANGE_IO_CHANNEL_FSCHANNEL_H_

#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <functional>

#include <xchange/base/Mutex.h>
#include <xchange/base/ThreadPool.h>
#include <xchange/io/Buffer.h>
#include <xchange/io/Channel.h>

namespace xchange {
namespace io {
namespace channel {

    class FSChannel: public Channel {
        public:
            typedef std::function<void(int error, const Buffer &)> ReadCallback;
            typedef std::function<void(int error, void *userData)> WriteCallback;

            struct ReadRequest {
                ReadRequest(int f, const ReadCallback &cb, xchange::base::Mutex &lock)
                    : fd(f), callback(cb), error(0), fileLock(lock) {}
                const int fd;
                int type;
                ReadCallback callback;
                Buffer buff;
                int error;
                xchange::base::Mutex &fileLock;
            };
            struct WriteRequest {
                WriteRequest(int f, const WriteCallback &cb, xchange::base::Mutex &lock, void *ud = NULL)
                    : fd(f), callback(cb), error(0), userData(ud), fileLock(lock) {}
                const int fd;
                int type;
                WriteCallback callback;
                Buffer buff;
                int error;
                void *userData;
                xchange::base::Mutex &fileLock;
            };

            static FSChannel *open(xchange::threadPool::ThreadPool::ptr &pool, const char *path, int flags, mode_t mode);

            FSChannel(int fd, xchange::threadPool::ThreadPool::ptr &pool, int flags);
            ~FSChannel() {close();}

            int fd() const {return fd_;}
            ChannelType type() const {return ChannelType::FS;}

            int64_t read(uint64_t size, const ReadCallback &readCallback);
            int64_t write(const Buffer &wbuff, const WriteCallback &writeCallback, void *userData = NULL);
            off_t seek(off_t offset, int whence);

            void close();
        private:
            const int fd_;
            xchange::threadPool::ThreadPool::ptr pool_;
            xchange::base::Mutex mutex_;
    };

}
}
}

#endif
