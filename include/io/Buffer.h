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
            explicit Buffer(uint64_t size = 0): data_(size == 0 ? NULL : new uint8_t[size]), size_(size){};
            Buffer(const Buffer & buff);
            Buffer(Buffer && buff);
            Buffer(const char *str);
            explicit Buffer(const char *str, uint64_t len);
            explicit Buffer(const uint8_t *str, uint64_t len);
            Buffer(const std::string & str);
            ~Buffer();

            uint8_t operator[](uint64_t pos) const {return *(data_+pos);};
            Buffer operator+(const Buffer &buff) const;
            Buffer& operator+=(const Buffer &buff);
            uint8_t get(uint64_t pos) const {return data_[pos];};
            Buffer slice(uint64_t pos, uint64_t len) const;


            void write(uint64_t pos, const Buffer &buff) {
                memcpy(data_+pos, buff.data_, buff.size_);
            }
            void write(uint64_t pos, const uint8_t* data, uint64_t len) {
                memcpy(data_+pos, data, len);
            }

            Buffer read(uint64_t pos, uint64_t len) const {
                Buffer temp(data_+pos, len);
                return temp;
            }

            void destroy() {delete this;};

            uint64_t size() const {return size_;};
            const uint8_t *data() const {return data_;};
            uint8_t *expose() {return data_;};

            friend std::ostream & operator<<(std::ostream &out, Buffer &buff) {
                if (buff.size_ == 0) {
                    return out << "(empty Buffer)";
                }

                out << buff.data();

                return out;
            }

            friend std::ostream & operator<<(std::ostream &out, Buffer &&buff) {
                if (buff.size_ == 0) {
                    return out << "(empty Buffer)";
                }

                out << buff.data();

                return out;
            }
        private:
            uint8_t *data_;
            uint64_t size_;
    };

}

}

#endif
