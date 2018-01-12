#include <xchange/net/Address.h>
#include <iostream>

using std::cout;
using std::endl;
using xchange::net::SocketAddress;

int main() {
    cout << SocketAddress(SocketAddress::SockType::IPv4, "222.197.181.4", 2222).toString() << endl;
    cout << SocketAddress(SocketAddress::SockType::IPv6, "::1", 123).toString() << endl;
    cout << SocketAddress(SocketAddress::SockType::IPv6, "ffff:ffff::fcda:1", 123).toString() << endl;

    SocketAddress addr(SocketAddress::SockType::IPv6, "rfff::1", 222);

    if (!addr.valid()) {
        cout << "invalid ip address from valid" << endl;
    }

    if (addr.toString().size() == 0) {
        cout << "invalid ip address from toString" << endl;
    }

    return 0;
}

