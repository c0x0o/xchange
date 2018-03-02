#ifndef XCHANGE_IO_CHANNEL_TCPCHANNEL_H_
#define XCHANGE_IO_CHANNEL_TCPCHANNEL_H_

#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <xchange/io/Buffer.h>
#include <xchange/io/Channel.h>

namespace xchange {
namespace io {
namespace channel {

    class TCPChannel: public Channel {
        public:
            static TCPChannel *connect(int af_family,
                                       const struct sockaddr *peer, uint64_t peerlen,
                                       const struct sockaddr *host = NULL, uint64_t hostlen = 0);

            TCPChannel(int fd, const struct sockaddr *peer, uint64_t socklen);
            ~TCPChannel() {close();}

            int fd() const {return fd_;}
            ChannelType type() const {return ChannelType::TCP;}
            const struct sockaddr* getHostSockaddr() const {return (struct sockaddr *)(&host_);}
            const struct sockaddr* getPeerSockaddr() const {return (struct sockaddr *)(&peer_);}

            Buffer read(uint64_t size);
            uint64_t write(const Buffer &wbuff);

            void close();
        private:
            const int fd_;
            struct sockaddr_storage host_, peer_;
    };

}
}
}

#endif
