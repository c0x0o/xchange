#include <xchange/net/Address.h>

using xchange::net::SocketAddress;

SocketAddress::SocketAddress(
        SockType type,
        const std::string &addr,
        uint16_t port
        ): type_(type), valid_(true) {
    if (type_ == SockType::IPv4) {
        struct sockaddr_in *ip = (struct sockaddr_in *)(&storage_);

        int ret = inet_pton(AF_INET, addr.c_str(), &(ip->sin_addr));
        if (ret <= 0) {
            // invalid address
            valid_ = false;
            return;
        }

        ip->sin_family = AF_INET;
        ip->sin_port = htons(port);
    } else if (type == SockType::IPv6) {
        struct sockaddr_in6 *ip = (struct sockaddr_in6 *)(&storage_);

        int ret = inet_pton(AF_INET6, addr.c_str(), &(ip->sin6_addr));
        if (ret <= 0) {
            valid_ = false;
            return;
        }

        ip->sin6_family = AF_INET6;
        ip->sin6_port = htons(port);
    } else {
        valid_ = false;
    }
}

SocketAddress::SocketAddress(SockType type,
        const struct sockaddr_storage *storage): type_(type), valid_(false) {
    memcpy(&storage_, storage, sizeof(struct sockaddr_storage));
}

SocketAddress::SocketAddress(const struct sockaddr *addr)
    :type_(SockType::IPv4), valid_(false) {
    memcpy(&storage_, addr, sizeof(struct sockaddr));
}

SocketAddress::SocketAddress(const struct sockaddr_in *addr)
    :type_(SockType::IPv4), valid_(false) {
    memcpy(&storage_, addr, sizeof(struct sockaddr_in));
}

SocketAddress::SocketAddress(const struct sockaddr_in6 *addr)
    :type_(SockType::IPv6), valid_(false) {
    memcpy(&storage_, addr, sizeof(struct sockaddr_in6));
}

const std::string SocketAddress::toString() const {
    char buffer[INET6_ADDRSTRLEN+8] = {0};

    if (!valid_) return std::string();

    if (type_ == IPv4) {
        struct sockaddr_in *ip = (struct sockaddr_in *)(&storage_);

        if (inet_ntop(AF_INET, &(ip->sin_addr), buffer, INET6_ADDRSTRLEN+8) != NULL) {
            sprintf(buffer+strlen(buffer), ":%d", ntohs(ip->sin_port));
        } else {
            return std::string();
        }

        return std::string(buffer);
    } else if (type_ == IPv6) {
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
