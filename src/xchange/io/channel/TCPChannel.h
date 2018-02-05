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
            static TCPChannel *createTCPChannel(int af_family, const struct sockaddr *addr, uint64_t socklen);

            TCPChannel(int fd, const struct sockaddr *addr, uint64_t socklen);
            ~TCPChannel() {close();}

            int fd() const {return fd_;}
            ChannelType type() const {return ChannelType::TCP;}
            const struct sockaddr* getSockaddr() const {return (struct sockaddr *)(&addr_);}

            Buffer read(uint64_t size);
            uint64_t write(const Buffer &wbuff);

            void close();
        private:
            const int fd_;
            struct sockaddr_storage addr_;
    };

}
}
}

#endif
