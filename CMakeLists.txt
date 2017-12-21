cmake_minimum_required (VERSION 2.6)

project(xchange)
set(CMAKE_BUILD_TYPE debug)

include_directories(include)

set(THREAD_SOURCE ./src/base/Thread.cc)
set(THREADPOOL_SOURCE ./src/base/Thread.cc ./src/base/ThreadPool.cc)
set(BUFFER_SOURCE ./src/io/Buffer.cc)
set(CACHE_SOURCE ./src/io/Cache.cc)

add_executable(threadTest ${THREAD_SOURCE} ./test/thread.cc)
add_executable(queueTest ${THREAD_SOURCE} ./test/LockFreeQueue.cc)
add_executable(threadPoolTest ${THREADPOOL_SOURCE} ./test/threadPool.cc)
add_executable(bufferTest ${BUFFER_SOURCE} ./test/buffer.cc)
add_executable(cacheTest ${BUFFER_SOURCE} ${CACHE_SOURCE} ./test/cache.cc)

target_link_libraries(threadTest pthread)
target_link_libraries(queueTest pthread)
target_link_libraries(threadPoolTest pthread)
