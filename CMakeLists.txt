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

set(PROTO_PATH "${CMAKE_SOURCE_DIR}/protos")
set(COMPANY_PROTO "${PROTO_PATH}/company.proto")
set(GENERATED_PROTOBUF_PATH "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${GENERATED_PROTOBUF_PATH})

# Generated grpc and proto sources
set(COMPANY_PROTO_HEADER "${GENERATED_PROTOBUF_PATH}/company.pb.h")
set(COMPANY_PROTO_SRC "${GENERATED_PROTOBUF_PATH}/company.pb.cc")
set(COMPANY_GRPC_HEADER "${GENERATED_PROTOBUF_PATH}/company.grpc.pb.h")
set(COMPANY_GRPC_SRC "${GENERATED_PROTOBUF_PATH}/company.grpc.pb.cc")

add_custom_command(
    OUTPUT "${COMPANY_PROTO_SRC}"
           "${COMPANY_PROTO_HEADER}"
    COMMAND /usr/local/bin/protoc
    ARGS -I "${PROTO_PATH}"
         --cpp_out="${GENERATED_PROTOBUF_PATH}"
         "${COMPANY_PROTO}"
    DEPENDS "${COMPANY_PROTO}"
)

add_custom_command(
    OUTPUT "${COMPANY_GRPC_SRC}"
           "${COMPANY_GRPC_HEADER}"
    COMMAND /usr/local/bin/protoc
    ARGS --grpc_out "${GENERATED_PROTOBUF_PATH}" -I "${PROTO_PATH}"
         --plugin=protoc-gen-grpc="/usr/local/bin/grpc_cpp_plugin"
         "${COMPANY_PROTO}"
    DEPENDS "${COMPANY_PROTO}"
)

set(SUBDIRS sync async)

foreach(dir ${SUBDIRS})
    add_subdirectory(${dir})
endforeach()
