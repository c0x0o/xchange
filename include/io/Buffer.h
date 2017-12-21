#ifndef _IO_BUFFER_H_
#define _IO_BUFFER_H_

#include <string.h>
#include <unistd.h>

#include <string>
#include <iostream>

namespace xchange {

namespace io {

    class Buffer {
        public:
            explicit Buffer(uint64_t size = 0): data_(new uint8_t[size]), size_(size){};
            Buffer(const Buffer & buff);
            Buffer(Buffer && buff);
            Buffer(const char *str);
            explicit Buffer(const char *str, uint64_t len);
            explicit Buffer(const uint8_t *str, uint64_t len);
            Buffer(const std::string & str);
            ~Buffer();

            uint8_t operator[](uint64_t pos) {return *(data_+pos);};

            Buffer& operator+(const Buffer &buff);

            Buffer& operator+=(const Buffer &buff);

            uint8_t get(uint64_t pos) {return data_[pos];};

            void write(uint64_t pos, const uint8_t* data, uint64_t len) {
                memcpy(data_+pos, data, len);
            }

            Buffer& read(uint64_t pos, uint64_t len) {
                return *(new Buffer(data_+pos, len));
            }

            void destroy() {delete this;};

            uint64_t size() const {return size_;};
            const uint8_t *data() const {return data_;};

            friend std::ostream & operator<<(std::ostream &out, Buffer &buff) {
                return out << buff.data();
            }
        private:
            uint8_t *data_;
            uint64_t size_;
    };

}

}

#endif
