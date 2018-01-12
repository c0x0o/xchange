#ifndef _NET_ADDRESS_H_
#define _NET_ADDRESS_H_

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#include <string>

namespace xchange {
namespace net {
    class SocketAddress;

    class SocketAddress {
        public:
            typedef enum {IPv4 = 0, IPv6} SockType;

            SocketAddress(SockType, const std::string & addr, uint16_t port);
            SocketAddress(SockType, const struct sockaddr_storage *);
            SocketAddress(const struct sockaddr *);
            SocketAddress(const struct sockaddr_in *);
            SocketAddress(const struct sockaddr_in6 *);
            ~SocketAddress() {};

            const struct sockaddr_in *getIpv4Struct() const {return (struct sockaddr_in *)&storage_;}
            const struct sockaddr_in6 *getIpv6Struct() const {return (struct sockaddr_in6 *)&storage_;}
            const std::string toString() const;

            bool isIpv4() const {return type_==IPv4;}
            bool isIpv6() const {return type_==IPv6;}
            bool valid() const {return valid_;}
        private:
            SockType type_;
            bool valid_;
            struct sockaddr_storage storage_;
    };

}
}

#endif
