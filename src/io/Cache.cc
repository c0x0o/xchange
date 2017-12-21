#include "io/Cache.h"

using xchange::io::Cache;
using xchange::io::Buffer;

Cache::Cache(uint64_t size)
    : data_(new uint8_t[size]),
    maxSize_(size),
    readIndex_(0),
    writeIndex_(0)
{
}

Cache::~Cache() {
    delete []data_;
}

Buffer& Cache::read(uint64_t len) {
    uint64_t currentSize = size();
    uint64_t start = readIndex_ % maxSize_, end = (readIndex_+len) % maxSize_;

    if (currentSize < len) {
        len = currentSize;
    }

    if (start <= end) {
        Buffer * res = (new Buffer(data_+start, len));

        readIndex_ += len;

        return *res;
    } else {
        Buffer * res = new Buffer(len);
        uint64_t firstPart = maxSize_ - start;

        res->write(0, data_+start, firstPart);
        res->write(firstPart, data_, end);

        readIndex_ += len;

        return *res;
    }
}

int Cache::write(const Buffer &buff) {
    uint64_t currentSize = size();
    uint64_t start = writeIndex_ % maxSize_, end = (writeIndex_+buff.size()) % maxSize_;

    if (maxSize_ < currentSize + buff.size()) {
        return 1;
    }

    if (start <= end) {
        memcpy(data_+start, buff.data(), buff.size());
    } else {
        uint64_t firstPart = maxSize_ - start;

        memcpy(data_+start, buff.data(), firstPart);
        memcpy(data_, buff.data()+firstPart, end);
    }

    writeIndex_ += buff.size();

    return 0;
}
