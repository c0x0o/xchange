#include <xchange/io/Buffer.h>

using xchange::io::Buffer;

// malloc size+1 to fix string like 'xxxx\0'

Buffer::Buffer(const Buffer & buff): size_(buff.size_) {
    data_ = new uint8_t[size_+1];
    data_[size_] = 0;

    memcpy(data_, buff.data_, size_);
}

Buffer::Buffer(Buffer && buff): size_(buff.size_) {
    data_ = buff.data_;
    buff.data_ = NULL;
}

Buffer::Buffer(const char *str) {
    size_ = strlen(str);
    data_ = new uint8_t[size_+1];
    data_[size_] = 0;

    memcpy(data_, str, size_);
}

Buffer::Buffer(const char *str, uint64_t len) {
    size_ = len;
    data_ = new uint8_t[size_+1];
    data_[size_] = 0;

    memcpy(data_, str, size_);
}

Buffer::Buffer(const uint8_t *str, uint64_t len) {
    size_ = len;
    data_ = new uint8_t[size_+1];
    data_[size_] = 0;

    memcpy(data_, str, size_);
}

Buffer::Buffer(const std::string & str) {
    new(this) Buffer(str.c_str());
}

Buffer::~Buffer() {
    if (data_ != NULL) {
        delete []data_;
    }
}

Buffer Buffer::operator+(const Buffer &buff) const {
    if (buff.size() == 0) {
        return Buffer(buff);
    }

    Buffer temp(buff.size_ + size_);

    memcpy(temp.data_, data_, size_);
    memcpy(temp.data_+size_, buff.data_, buff.size_);

    return temp;
}

Buffer& Buffer::operator+=(const Buffer &buff) {
    uint8_t *old = data_;

    if (buff.size() == 0) return *this;

    data_ = new uint8_t[size_+buff.size_+1];
    data_[size_+buff.size_] = 0;

    memcpy(data_, old, size_);
    memcpy(data_+size_, buff.data_, buff.size_);

    size_ = size_ + buff.size_;

    delete []old;

    return *this;
}

Buffer Buffer::slice(uint64_t pos, uint64_t len) const {
    return Buffer(data_+pos, len);
}


