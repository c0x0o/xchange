#include <xchange/io/Buffer.h>

using xchange::io::Buffer;

Buffer::Buffer(uint64_t size)
    :size_(size) {
    data_ = make_shared_array<uint8_t>(size_);
}

Buffer::Buffer(const Buffer & buff): size_(buff.size_) {
    if (size_ > 0) {
        data_ = make_shared_array<uint8_t>(size_);
        memcpy(data_.get(), buff.data_.get(), size_);
    } else {
        data_ = NULL;
    }
}

Buffer::Buffer(Buffer && buff): size_(buff.size_) {
    if (bool(data_)) {
        data_.reset();
    }

    data_.swap(buff.data_);
}

Buffer::Buffer(const char *str) {
    size_ = strlen(str)+1;
    data_ = make_shared_array<uint8_t>(size_);

    memcpy(data_.get(), str, size_);
}

Buffer::Buffer(const char *str, uint64_t len) {
    if (len == 0) {
        size_ = 0;
        data_ = NULL;
    } else {
        size_ = len;
        data_ = make_shared_array<uint8_t>(len);
        memcpy(data_.get(), str, len);
    }
}

Buffer::Buffer(const uint8_t *str, uint64_t len) {
    if (len == 0) {
        size_ = 0;
        data_ = NULL;
    } else {
        size_ = len;
        data_ = make_shared_array<uint8_t>(len);
        memcpy(data_.get(), str, len);
    }
}

Buffer::Buffer(const std::string & str) {
    new(this) Buffer(str.c_str());
}

Buffer::~Buffer() {
    if (bool(data_)) {
        data_.reset();
    }
}

Buffer Buffer::operator+(const Buffer &buff) const {
    if (buff.size() == 0) {
        return Buffer(buff);
    }

    Buffer temp(buff.size_ + size_);

    memcpy(temp.data_.get(), data_.get(), size_);
    memcpy(temp.data_.get()+size_, buff.data_.get(), buff.size_);

    return temp;
}

Buffer& Buffer::operator+=(const Buffer &buff) {
    if (buff.size() == 0) return *this;

    std::shared_ptr<uint8_t> old = make_shared_array<uint8_t>(size_+buff.size_);

    data_.swap(old);

    memcpy(data_.get(), old.get(), size_);
    memcpy(data_.get()+size_, buff.data_.get(), buff.size_);

    size_ = size_ + buff.size_;

    return *this;
}
Buffer& Buffer::operator=(const Buffer &buff) {
    if (buff.size() == 0) {
        data_.reset();
    } else {
        data_ = make_shared_array<uint8_t>(buff.size_);
        size_ = buff.size_;

        memcpy(data_.get(), buff.data_.get(), buff.size_);
    }

    return *this;
}
Buffer& Buffer::operator=(Buffer &&buff) {
    data_ = buff.data_;
    buff.data_ = NULL;

    return *this;
}

Buffer Buffer::slice(uint64_t pos, uint64_t len) const {
    return Buffer(data_.get()+pos, len);
}

void Buffer::own(char *str, uint64_t len) {
    data_ = std::shared_ptr<uint8_t>((uint8_t *)str);
    size_ = len;
}
void Buffer::own(uint8_t *str, uint64_t len) {
    data_ = std::shared_ptr<uint8_t>(str);
    size_ = len;
}

Buffer &Buffer::share(const Buffer &src) {
    data_ = src.data_;
    size_ = src.size_;

    return *this;
}

