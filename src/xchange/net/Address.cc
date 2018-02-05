#include <xchange/net/Address.h>

using xchange::net::socket::SocketAddress;

SocketAddress::SocketAddress(
        const std::string &addr,
        uint16_t port,
        SocketFamily type
        ): family_(type)
{
        struct sockaddr_in *ip = (struct sockaddr_in *)(&storage_);

        int ret = inet_pton(type, addr.c_str(), &(ip->sin_addr));
        if (ret <= 0) {
            // invalid address
            family_ = invalid;
            return;
        }

        ip->sin_family = type;
        ip->sin_port = htons(port);
}

SocketAddress::SocketAddress(
        const struct sockaddr_storage *storage,
        SocketFamily type
        ): family_(type)
{
    memcpy(&storage_, storage, sizeof(struct sockaddr_storage));
}

SocketAddress::SocketAddress(const struct sockaddr_in *addr)
    :family_(SocketFamily::ipv4)
{
    memcpy(&storage_, addr, sizeof(struct sockaddr_in));
}

SocketAddress::SocketAddress(const struct sockaddr_in6 *addr)
    :family_(SocketFamily::ipv6)
{
    memcpy(&storage_, addr, sizeof(struct sockaddr_in6));
}

SocketAddress::SocketFamily SocketAddress::checkAddressFamily(const std::string &addr) {
    if (addr.find(":") == std::string::npos) {
        return ipv4;
    }

    return ipv6;
};

const std::string SocketAddress::toString() const {
    char buffer[INET6_ADDRSTRLEN+8] = {0};

    if (family_ == invalid) return std::string();

    if (family_ == ipv4) {
        struct sockaddr_in *ip = (struct sockaddr_in *)(&storage_);

        if (inet_ntop(AF_INET, &(ip->sin_addr), buffer, INET6_ADDRSTRLEN+8) != NULL) {
            sprintf(buffer+strlen(buffer), ":%d", ntohs(ip->sin_port));
        } else {
            return std::string();
        }

        return std::string(buffer);
    } else if (family_ == ipv6) {
        struct sockaddr_in6 *ip = (struct sockaddr_in6 *)(&storage_);

        buffer[0] = '[';
        if (inet_ntop(AF_INET6, &(ip->sin6_addr), buffer+1, INET6_ADDRSTRLEN+8) != NULL) {
            sprintf(buffer+strlen(buffer), "]:%d", ntohs(ip->sin6_port));
        } else {
            return std::string();
        }

        return std::string(buffer);
    }

    // return an empty string when error
    return std::string();
}
