#ifndef XCHANGE_IO_CHANNEL_PTCPCHANNEL_H_
#define XCHANGE_IO_CHANNEL_PTCPCHANNEL_H_

#include <netinet/in.h>

#include <xchange/io/Channel.h>
#include <xchange/io/channel/TCPChannel.h>

namespace xchange {
namespace io {
namespace channel {

    class PTCPChannel: public Channel {
        public:
            static PTCPChannel *createPassiveTCPChannel(int af_family, const struct sockaddr *addr, uint64_t socklen);

            ~PTCPChannel() {close();}

            int fd() const {return fd_;}
            ChannelType type() const {return ChannelType::PTCP;}
            const struct sockaddr* getSockaddr() const {return (struct sockaddr *)&addr_;}

            TCPChannel* accept();

            void close();
        private:
            PTCPChannel(int fd, const struct sockaddr *addr, uint64_t socklen);

            const int fd_;
            struct sockaddr_storage addr_;
    };

}
}
}

#endif
