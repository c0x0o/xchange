cmake_minimum_required (VERSION 2.6)

project(xchange)
set(CMAKE_BUILD_TYPE debug)

include_directories(include)

set(BASE_SOURCE ./src/base/Thread.cc ./src/base/ThreadPool.cc)

add_executable(threadTest ${BASE_SOURCE} ./test/thread.cc)
add_executable(queueTest ${BASE_SOURCE} ./test/LockFreeQueue.cc)
add_executable(threadPoolTest ${BASE_SOURCE} ./test/threadPool.cc)

target_link_libraries(threadTest pthread)
target_link_libraries(queueTest pthread)
target_link_libraries(threadPoolTest pthread)
