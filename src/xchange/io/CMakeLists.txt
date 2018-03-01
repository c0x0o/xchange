set(IO_SOURCE
    "./utils.cc"
    "./Buffer.cc"
    "./Cache.cc"
    "./EventLoop.cc"
    "./channel/FSChannel.cc"
    "./channel/TCPChannel.cc"
    "./channel/PTCPChannel.cc"
    "./poller/EpollPoller.cc")

set(IO_HEADER
    "./utils.h"
    "./Buffer.h"
    "./Cache.h"
    "./Channel.h"
    "./EventLoop.h"
    "./Poller.h"
    "./channel/FSChannel.h"
    "./channel/TCPChannel.h"
    "./channel/PTCPChannel.h"
    "./poller/EpollPoller.h")


add_library(io ${IO_SOURCE})
target_link_libraries(io base)
target_link_libraries(io algorithm)
target_link_libraries(io design)
target_link_libraries(io pthread)

install(TARGETS io DESTINATION lib/xchange)
install(FILES ${IO_HEADER} DESTINATION include/xchange/io)
