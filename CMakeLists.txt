cmake_minimum_required (VERSION 2.6)

project(xchange)
set(CMAKE_BUILD_TYPE debug)

include_directories(AFTER SYSTEM src)

set(THREAD_SOURCE ./src/xchange/base/Thread.cc)
set(THREADPOOL_SOURCE ./src/xchange/base/Thread.cc ./src/xchange/base/ThreadPool.cc)
set(BUFFER_SOURCE ./src/xchange/io/Buffer.cc)
set(CACHE_SOURCE ./src/xchange/io/Cache.cc)
set(IOMANAGER_SOURCE ./src/xchange/io/IOManager.cc ./src/xchange/io/EpollManager.cc)
set(TIMER_SOURCE ./src/xchange/base/Timer.cc)
set(NET_SOURCE ./src/xchange/net/Address.cc)

add_executable(threadTest ${THREAD_SOURCE} ./test/thread.cc)
add_executable(queueTest ${THREAD_SOURCE} ./test/LockFreeQueue.cc)
add_executable(threadPoolTest ${THREADPOOL_SOURCE} ./test/threadPool.cc)
add_executable(bufferTest ${BUFFER_SOURCE} ./test/buffer.cc)
add_executable(cacheTest ${BUFFER_SOURCE} ${CACHE_SOURCE} ./test/cache.cc)
add_executable(rbtree ./test/rbtree.cc)
add_executable(epoll ${BUFFER_SOURCE} ${CACHE_SOURCE} ${IOMANAGER_SOURCE} ./test/epoll.cc)
add_executable(timer ${TIMER_SOURCE} ${THREAD_SOURCE} ./test/timer.cc)
add_executable(address ${NET_SOURCE} ./test/address.cc)

target_link_libraries(threadTest pthread)
target_link_libraries(queueTest pthread)
target_link_libraries(threadPoolTest pthread)
target_link_libraries(timer pthread)
