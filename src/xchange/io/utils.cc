#include <xchange/io/utils.h>

using xchange::io::channel::Channel;
using xchange::io::utils::setNonblockingChannel;

int xchange::io::utils::setNonblockingChannel(const Channel *channel) {
    int fl = ::fcntl(channel->fd(), F_GETFL) | O_NONBLOCK;

    return ::fcntl(channel->fd(), F_SETFL, fl);
}
