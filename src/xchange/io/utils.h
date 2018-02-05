#ifndef XCHANGE_IO_UTILS_H_
#define XCHANGE_IO_UTILS_H_

#include <fcntl.h>

#include <xchange/io/Channel.h>

namespace xchange {
namespace io {
namespace utils {
    int setNonblockingChannel(const xchange::io::channel::Channel *channel);
}
}
}

#endif
