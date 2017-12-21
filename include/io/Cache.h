#ifndef _IO_CACHE_H_
#define _IO_CACHE_H_

#include "io/Buffer.h"
#include "base/EventEmitter.h"

#define mod(a,b) (a&(b-1))

namespace xchange {

namespace io {

    class Cache {
        public:
            Cache(uint64_t size);
            ~Cache();

            Buffer& read(uint64_t len);
            int write(const Buffer &buff);

            uint64_t size() const {return mod(writeIndex_, 64) - mod(readIndex_, 64);};
        private:
            uint8_t *data_;
            uint64_t maxSize_;
            uint64_t readIndex_, writeIndex_;
    };

}

}

#endif
