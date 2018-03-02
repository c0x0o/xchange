#include <xchange/io/channel/TCPChannel.h>

#include <arpa/inet.h>
#include <stdio.h>

#include <iostream>
#include <string>

#include <xchange/io/EventLoop.h>
#include <xchange/io/poller/EpollPoller.h>
#include <xchange/io/Channel.h>
#include <xchange/io/utils.h>

using std::cin;
using std::cout;
using std::endl;
using xchange::threadPool::ThreadPool;
using xchange::io::Buffer;
using xchange::io::EventLoop;
using xchange::io::channel::TCPChannel;
using xchange::io::poller::EpollPoller;
using xchange::io::channel::ChannelEvent;


int main() {
    ThreadPool::ptr poolptr(new ThreadPool());
    EpollPoller::ptr pollerptr(new EpollPoller());
    EventLoop loop(poolptr, pollerptr);

    struct sockaddr_in peer;
    int ret;

    peer.sin_family = AF_INET;
    ret = inet_pton(AF_INET, "127.0.0.1", &peer.sin_addr);
    if (ret != 1) {
        cout << "create socket struct failed: " << strerror(errno) << endl;
        return 1;
    }
    peer.sin_port = htons(12345);

    TCPChannel *connector = TCPChannel::connect(AF_INET, (struct sockaddr *)&peer, sizeof(struct sockaddr_in));
    if (connector == NULL) {
        cout << "connect to peer failed: " << strerror(errno) << endl;
        return 0;
    }

    const std::function<void()> receiveUserInput = [connector, &loop]()->void {
        std::string input;

        if (connector->eof() || connector->error()) {
            cout << "exit client..." << endl;
            loop.stop();
            return;
        }

        cout << "input your data: ";

        cin >> input;
        if (input == "exit") {
            cout << "exit client..." << endl;
            loop.stop();
            return;
        }

        if (connector->writeable()) {
            Buffer buff(input.c_str());

            connector->write(buff);
        } else {
            cout << "socket is currently unwritable. exit..." << endl;
        }
    };

    connector->on(ChannelEvent::IN, [&loop, &receiveUserInput](ChannelEvent, void *c) {
                TCPChannel *conn = (TCPChannel *)c;

                while (conn->readable()) {
                    const Buffer &result = conn->read(1024);

                    if (result.size() > 0) {
                        cout << result;
                    } else {
                        break;
                    }
                }

                if (conn->eof()) {
                    cout << "connection closed by peer" << endl;
                }

                cout << endl;

                loop.runInLoop(receiveUserInput);
            });
    loop.runInLoop(receiveUserInput);

    xchange::io::utils::setNonblockingChannel(connector);

    loop.addChannel(connector);
    loop.loop();

    return 0;
}
