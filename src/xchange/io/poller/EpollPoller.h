#ifndef XCHANGE_IO_POLLER_EPOLLPOLLER_H_
#define XCHANGE_IO_POLLER_EPOLLPOLLER_H_

#include <unistd.h>
#include <sys/epoll.h>

#include <xchange/io/Channel.h>
#include <xchange/io/Poller.h>

namespace xchange {
namespace io {
namespace poller {

    class EpollPoller: public Poller {
        public:
            EpollPoller();
            ~EpollPoller();

            int poll(uint64_t timeoutMs);
            int bind(channel::Channel *channel);
            int unbind(channel::Channel *channel);
            int updateChannelEvent(channel::Channel &channel);
        private:
            int epollfd_;
    };
}
}
}

#endif
