#include <xchange/io/channel/PTCPChannel.h>

using xchange::io::channel::PTCPChannel;
using xchange::io::channel::TCPChannel;
using xchange::io::Buffer;

PTCPChannel* PTCPChannel::createPassiveTCPChannel(
        int af_family,
        const struct sockaddr *addr,
        uint64_t socklen)
{
    int fd = ::socket(af_family, SOCK_STREAM, 0);
    int ret;

    if (fd < 0) {
        return NULL;
    }

    ret = ::bind(fd, addr, socklen);
    if (ret < 0) {
        ::close(fd);
        return NULL;
    }

    ret = ::listen(fd, 1000);
    if (ret < 0) {
        ::close(fd);
        return NULL;
    }

    return new PTCPChannel(fd, addr, socklen);
}

PTCPChannel::PTCPChannel(
        int fd,
        const struct sockaddr *addr,
        uint64_t socklen)
    : Channel(IN | ERROR, false, false, false),
    fd_(fd)
{
    memset(&addr_, 0, sizeof(struct sockaddr_storage));
    memcpy(&addr_, addr, socklen);
}

TCPChannel* PTCPChannel::accept() {
    int connfd;
    struct sockaddr_storage addr;
    socklen_t socklen;

    if (!readable_ || error_) {
        return NULL;
    }

AGAIN:
    connfd = ::accept(fd_, (struct sockaddr *)&addr, &socklen);
    if (connfd < 0) {
        if (errno == EINTR) {
            goto AGAIN;
        }

        readable_ = false;

        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            error_ = true;
        }

        return NULL;
    }

    return new TCPChannel(connfd, (struct sockaddr *)&addr, socklen);
}

void PTCPChannel::close() {
    readable_ = false;
    writeable_ = false;
    ::close(fd_);
}
