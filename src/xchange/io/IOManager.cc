#include <xchange/io/IOManager.h>
#include <xchange/io/Cache.h>
#include <xchange/io/Buffer.h>

using xchange::EventEmitter;
using xchange::io::IOContext;
using xchange::io::Cache;
using xchange::io::Buffer;

IOContext::IOContext(int watchfd, int evts, uint64_t cacheSize)
    : fd_(watchfd),
    events_(evts),
    readable_(false),
    writeable_(true),
    fatal_(false),
    readCache_(cacheSize),
    writeCache_(cacheSize)
{
}
IOContext::~IOContext() {}

#define CHUNK_SIZE (1024*8)

int IOContext::readIntoCache_() {
    uint64_t finalRead = 0, tempSize = readCache_.maxSize() < CHUNK_SIZE ? CHUNK_SIZE:readCache_.maxSize();
    uint8_t *temp = new uint8_t[tempSize];

    if (!readable_) {
        delete []temp;
        return -1;
    }

    while (1) {
        uint64_t canRead = readCache_.spareSize() < CHUNK_SIZE ? readCache_.spareSize() : CHUNK_SIZE;
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

int IOContext::writeFromCache_(bool needFlush) {
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

Buffer IOContext::read(uint64_t length) {
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

int64_t IOContext::write(const Buffer & content) {
    uint64_t nwrite = writeCache_.spareSize();;

    if (!writeable_) return -1;

    if (nwrite >= content.size()) {
        writeCache_.write(content);

        return (int64_t)content.size();
    }

    writeFromCache_();

    nwrite = 0;
    do {
        uint64_t usable = writeCache_.spareSize();
        uint64_t needWrite = content.size()-nwrite > usable ? usable : content.size()-nwrite;

        writeCache_.write(content.slice(nwrite, needWrite));

        if (writeCache_.spareSize() == 0) {
            writeFromCache_();
        }

        nwrite += needWrite;
    } while (writeable_ && nwrite < content.size());

    return (uint64_t)nwrite;
}

void IOContext::flush() {
    writeFromCache_(true);
}

