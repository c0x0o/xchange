set(NET_SOURCE
    "./Address.cc")

set(NET_HEADER
    "./Address.h")

add_library(net ${NET_SOURCE})

install(TARGETS net DESTINATION lib/xchange)
install(FILES ${NET_HEADER} DESTINATION include/xchange/net)
