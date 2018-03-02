#include <xchange/io/channel/TCPChannel.h>

using xchange::io::channel::TCPChannel;
using xchange::io::Buffer;

TCPChannel *TCPChannel::connect(
        int af_family,
        const struct sockaddr *peer, uint64_t peerlen,
        const struct sockaddr *host, uint64_t hostlen)
{
    int fd = ::socket(af_family, SOCK_STREAM, 0);
    int ret;

    if (fd < 0) {
        return NULL;
    }

    if (host != NULL) {
        ret = ::bind(fd, host, hostlen);
        if (ret < 0) {
            ::close(fd);
            return NULL;
        }
    }

    ret = ::connect(fd, peer, peerlen);
    if (ret != 0) {
        ::close(fd);
        return NULL;
    }

    return new TCPChannel(fd, peer, peerlen);
}

TCPChannel::TCPChannel(
        int fd,
        const struct sockaddr *peer,
        uint64_t socklen)
    :Channel(IN | ERROR, false, true, false),
    fd_(fd)
{
    socklen_t len = sizeof(struct sockaddr_storage);
    memset(&peer_, 0, len);
    memcpy(&peer_, peer, socklen);

    ::getsockname(fd, (struct sockaddr *)&host_, &len);
}

Buffer TCPChannel::read(uint64_t size) {
    uint64_t nread = 0, nleft = size;
    uint8_t *buff = new uint8_t[size];

    if (!readable_ || error_) {
        return Buffer();
    }

    while (nleft > 0) {
        int ret = ::read(fd_, buff+nread, nleft);

        if (ret < 0) {
            if (ret == EINTR) {
                continue;
            }
            // EPIPE may caused by shutdown
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EPIPE) {
                readable_ = false;

                if (errno == EPIPE) {
                    eof_ = true;
                }
            } else {
                readable_ = false;
                writeable_ = false;
                error_ = true;
            }
            break;
        }

        if (ret == 0) {
            eof_ = true;
            readable_ = false;
            break;
        }

        nread += ret;
        nleft -= ret;
    }

    Buffer buffer;
    buffer.own(buff, nread);

    return buffer;
}

uint64_t TCPChannel::write(const Buffer &wbuff) {
    uint64_t nwrite = 0, nleft = wbuff.size();

    if (!writeable_ || error_) {
        return 0;
    }

    while (nleft > 0) {
        int ret = ::write(fd_, wbuff.data()+nwrite, nleft);

        if (ret < 0) {
            if (ret == EINTR) {
                continue;
            }
            // EPIPE may caused by shutdown
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EPIPE) {
                writeable_ = false;
            } else {
                readable_ = false;
                writeable_ = false;
                error_ = true;
            }
            break;
        }

        nwrite += ret;
        nleft -= ret;
    }

    return nwrite;
}

void TCPChannel::close() {
    readable_ = false;
    writeable_ = false;
    ::close(fd_);
}
