set(BASE_SOURCE
    "./Condition.cc"
    "./Mutex.cc"
    "./Thread.cc"
    "./ThreadPool.cc"
    "./Timer.cc"
    )

set(BASE_HEADER
    "./Condition.h"
    "./EventEmitter.h"
    "./Mutex.h"
    "./Thread.h"
    "./ThreadPool.h"
    "./Timer.h"
    "./Timestamp.h")

add_library(base ${BASE_SOURCE})
target_link_libraries(base pthread)

install(TARGETS base DESTINATION lib/xchange)
install(FILES ${BASE_HEADER} DESTINATION include/xchange/base)
