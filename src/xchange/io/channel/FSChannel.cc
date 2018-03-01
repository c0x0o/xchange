#include <xchange/io/channel/FSChannel.h>
#include <iostream>

using xchange::io::channel::FSChannel;
using xchange::threadPool::ThreadPool;
using xchange::threadPool::Task;
using xchange::threadPool::TaskEvent;
using xchange::threadPool::TASK_COMPLETE;

FSChannel *FSChannel::open(
        xchange::threadPool::ThreadPool::ptr &pool,
        const char *path,
        int flags,
        mode_t mode)
{
    int fd;

    fd = ::open(path, flags, mode);
    if (fd < 0) {
        return NULL;
    }
    return new FSChannel(fd, pool, flags);
}

FSChannel::FSChannel(int fd, ThreadPool::ptr &pool, int flags)
    :Channel(IN|OUT|ERROR),
    fd_(fd),
    pool_(pool)
{
    if (flags & O_RDWR) {
        setWriteStatus(true);
    } else if (flags & O_WRONLY) {
        setReadStatus(false);
        setWriteStatus(true);
    }

    if (!pool->running()) {
        pool->start();
    }
}

off_t FSChannel::seek(off_t offset, int whence) {
    return ::lseek(fd_, offset, whence);
}

int64_t FSChannel::read(uint64_t size, const ReadCallback &readCallback) {
    if (size == 0) {
        return EINVAL;
    }

    ReadRequest *req = new ReadRequest(fd_, readCallback, mutex_);
    req->buff.resize(size);

    Task *taskp = new Task([](void *arg)->void * {
            ReadRequest *req = static_cast<ReadRequest *>(arg);
            uint8_t *buffer = new uint8_t[req->buff.size()];

            req->fileLock.lock();
            int64_t nread = ::read(req->fd, buffer, req->buff.size());
            req->fileLock.unlock();

            if (nread < 0) {
                req->error = errno;
            }

            req->buff.own(buffer, nread);

            return req;
        }, req, true);

    taskp->on(TASK_COMPLETE, [](TaskEvent, void *arg) {
                Task *taskp = static_cast<Task *>(arg);
                ReadRequest *req = static_cast<ReadRequest *>(taskp->getResult());

                if (bool(req->callback)) {
                    req->callback(req->error, req->buff);
                }

                delete req;
            });

    pool_->execute(taskp);

    return 0;
}

int64_t FSChannel::write(const Buffer &wbuff, const WriteCallback &readCallback, void *userData) {
    if (wbuff.size() == 0) {
        return 0;
    }

    WriteRequest *req = new WriteRequest(fd_, readCallback, mutex_, userData);
    req->buff.share(wbuff);

    Task *taskp = new Task([](void *arg)->void * {
            WriteRequest *req = static_cast<WriteRequest *>(arg);
            const Buffer &buffer = req->buff;

            req->fileLock.lock();
            int64_t nwrite = ::write(req->fd, buffer.data(), buffer.size());
            req->fileLock.unlock();

            if (nwrite < 0) {
                req->error = errno;
            }

            return req;
        }, req, true);

    taskp->on(TASK_COMPLETE, [](TaskEvent, void *arg) {
                Task *taskp = static_cast<Task *>(arg);
                WriteRequest *req = static_cast<WriteRequest *>(taskp->getResult());

                if (bool(req->callback)) {
                    req->callback(req->error, req->userData);
                }

                delete req;
            });

    pool_->execute(taskp);

    return wbuff.size();
}

void FSChannel::close() {
    ::close(fd_);

    mutex_.unlock();
}
