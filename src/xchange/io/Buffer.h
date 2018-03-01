#ifndef XCHANGE_IO_BUFFER_H_
#define XCHANGE_IO_BUFFER_H_

#include <string.h>
#include <unistd.h>
#include <memory>

#include <string>
#include <iostream>

namespace xchange {

namespace io {

    template<class T>
    std::shared_ptr<T> make_shared_array(uint64_t size) {
        return std::shared_ptr<T>(new T[size], std::default_delete<T[]>());
    }

    class Buffer {
        public:
            explicit Buffer(uint64_t size = 0);
            Buffer(const Buffer & buff);
            Buffer(Buffer && buff);
            Buffer(const char *str);
            explicit Buffer(const char *str, uint64_t len);
            explicit Buffer(const uint8_t *str, uint64_t len);
            Buffer(const std::string & str);
            ~Buffer();

            uint8_t operator[](uint64_t pos) const {return *(data_.get()+pos);};
            uint8_t get(uint64_t pos) const {return *(data_.get()+pos);};
            Buffer operator+(const Buffer &buff) const;
            Buffer& operator+=(const Buffer &buff);
            Buffer& operator=(const Buffer &buff);
            Buffer& operator=(Buffer &&buff);
            Buffer slice(uint64_t pos, uint64_t len) const;

            void write(uint64_t pos, const Buffer &buff) {
                memcpy(data_.get()+pos, buff.data_.get(), buff.size_);
            }
            void write(uint64_t pos, const uint8_t* data, uint64_t len) {
                memcpy(data_.get()+pos, data, len);
            }
            void write(uint64_t pos, const int8_t* data, uint64_t len) {
                memcpy(data_.get()+pos, data, len);
            }

            Buffer read(uint64_t pos, uint64_t len) const {
                Buffer temp(data_.get()+pos, len);
                return temp;
            }

            void resize(uint64_t newSize) {size_ = newSize;}
            void own(char *str, uint64_t len);
            void own(uint8_t *str, uint64_t len);
            Buffer &share(const Buffer &src);

            uint64_t size() const {return size_;}
            bool empty() const {return size_ == 0;}
            const uint8_t *data() const {return data_.get();}
            uint8_t *expose() {return data_.get();};

            friend std::ostream & operator<<(std::ostream &out, Buffer &buff) {
                if (buff.size_ == 0) {
                    return out;
                }

                out.write((char *)buff.data_.get(), buff.size_);

                return out;
            }

            friend std::ostream & operator<<(std::ostream &out, Buffer &&buff) {
                if (buff.size_ == 0) {
                    return out;
                }

                out.write((char *)buff.data_.get(), buff.size_);

                return out;
            }
        private:
            std::shared_ptr<uint8_t> data_;
            uint64_t size_;
    };

}

}

#endif
