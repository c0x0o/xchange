#include <unistd.h>

#include <iostream>

#include "io/EpollManager.h"
#include "io/Buffer.h"

using xchange::io::Buffer;
using xchange::io::EpollManager;
using xchange::io::IOContext;
using xchange::io::IOEvent;
using xchange::io::IO_READ;
using xchange::io::IO_WRITE;
using xchange::io::IO_ERROR;
using std::cout;
using std::endl;

// or you can seperate them into different callback
void handle_event(IOEvent evt, void *arg) {
    IOContext & ctx = *(IOContext *)arg;

    if (evt == IO_READ) {
        // when new data available in file
        // call 'EpollContextInstance.read' to get them

        // by calling without specific length, all data
        // available will be read
        Buffer buff = ctx.read();
        if (buff.size() > 0) {
            cout << buff;
        } else {
            cout << "nothing read" << endl;
        }
    } else if (evt == IO_WRITE) {
        // in most cases, fd is always writeable.
        // so you hardly need to listen this event,
        // just call 'EpollContextInstance.write' and
        // 'xchange' will help you cache your data
        cout << ctx.fd() << "turn to writeable from unwriteable" << endl;
    } else if (evt == IO_ERROR) {
        // handle error event
        // error like EPIPE will trigger this event.
        // these errors will also be delivered to fds
        // when you call read/write on them and they often cause
        // unexpected exit of the program,
        // so the recommended action is closing the fd.
        cout << ctx.fd() << "raised an error" << endl;
        close(ctx.fd());
    }
}

int main() {
    // default cache size for each fd is 16k, and default MAX_EVENT is 100
    EpollManager epoll;

    // listening 'IO_READ' event on standard input
    // noticed that every fd you commit to Epoll will be set unblocking
    epoll.watch(0);
    epoll.on(IO_READ, handle_event);

    while (1) {
        epoll.tick();
    }

    return 0;
}
