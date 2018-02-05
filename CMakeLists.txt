cmake_minimum_required (VERSION 3.0)

project(xchange)
set(CMAKE_BUILD_TYPE debug)
add_compile_options(-std=c++11)

include_directories(AFTER SYSTEM src)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_subdirectory(./src/xchange/algorithm)
add_subdirectory(./src/xchange/design)
add_subdirectory(./src/xchange/base)
add_subdirectory(./src/xchange/io)
add_subdirectory(./src/xchange/net)
add_subdirectory(./test EXCLUDE_FROM_ALL)
