#ifndef XCHANGE_IO_POLLER_H_
#define XCHANGE_IO_POLLER_H_

#include <memory>

#include <xchange/io/Channel.h>

namespace xchange {
namespace io {
namespace poller {
    class Poller {
        public:
            typedef std::shared_ptr<Poller> ptr;

            virtual ~Poller() {};

            virtual int poll(uint64_t timeoutMs = 0) = 0;

            virtual int bind(channel::Channel *channel) = 0;
            virtual int unbind(channel::Channel *channel) = 0;

            virtual int updateChannelEvent(channel::Channel &channel) = 0;
    };
}
}
}

#endif
