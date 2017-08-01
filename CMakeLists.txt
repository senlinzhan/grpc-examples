# Minimum CMake required
cmake_minimum_required(VERSION 3.3)

# Project
project(grpc-examples CXX)

# Pthreads
find_package(Threads REQUIRED)

# Protobuf
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

# CPP compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

set(SUBDIRS sync)

foreach(dir ${SUBDIRS})
    add_subdirectory(${dir})
endforeach()
