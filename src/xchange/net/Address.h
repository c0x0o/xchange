#ifndef _NET_ADDRESS_H_
#define _NET_ADDRESS_H_

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include <string>

namespace xchange {
namespace net {
namespace socket {
    class SocketAddress {
        public:
            typedef enum {invalid = AF_UNSPEC, ipv4 = AF_INET, ipv6 = AF_INET6, unix = AF_UNIX} SocketFamily;

            explicit SocketAddress(const std::string & addr, uint16_t port, SocketFamily family = ipv4);
            explicit SocketAddress(const struct sockaddr_storage *, SocketFamily family = ipv4);
            SocketAddress(const struct sockaddr_in *);
            SocketAddress(const struct sockaddr_in6 *);
            ~SocketAddress() {};

            static SocketFamily checkAddressFamily(const std::string &addr);

            const struct sockaddr *getStruct() const {return (struct sockaddr *)&storage_;}
            const struct sockaddr_in *getIpv4Struct() const {return (struct sockaddr_in *)&storage_;}
            const struct sockaddr_in6 *getIpv6Struct() const {return (struct sockaddr_in6 *)&storage_;}
            SocketFamily getSocketFamily() const {return family_;}

            const std::string toString() const;

            bool isIpv4() const {return family_ == ipv4;}
            bool isIpv6() const {return family_ == ipv6;}
            bool valid() const {return family_ == invalid;}
        private:
            SocketFamily family_;
            struct sockaddr_storage storage_;
    };
}
}
}

#endif
