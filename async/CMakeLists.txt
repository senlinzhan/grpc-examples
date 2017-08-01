set(PROTO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/protos")
set(GENERATED_PROTOBUF_PATH "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${GENERATED_PROTOBUF_PATH})

# Generated grpc and proto sources
set(HELLO_PROTO "${PROTO_PATH}/hello.proto")
set(HELLO_PROTO_HEADER "${GENERATED_PROTOBUF_PATH}/hello.pb.h")
set(HELLO_PROTO_SRC "${GENERATED_PROTOBUF_PATH}/hello.pb.cc")
set(HELLO_GRPC_HEADER "${GENERATED_PROTOBUF_PATH}/hello.grpc.pb.h")
set(HELLO_GRPC_SRC "${GENERATED_PROTOBUF_PATH}/hello.grpc.pb.cc")

add_custom_command(
    OUTPUT "${HELLO_PROTO_SRC}"
           "${HELLO_PROTO_HEADER}"
    COMMAND /usr/local/bin/protoc
    ARGS -I "${PROTO_PATH}"
         --cpp_out="${GENERATED_PROTOBUF_PATH}"
         "${HELLO_PROTO}"
    DEPENDS "${HELLO_PROTO}"
)

add_custom_command(
    OUTPUT "${HELLO_GRPC_SRC}"
           "${HELLO_GRPC_HEADER}"
    COMMAND /usr/local/bin/protoc
    ARGS --grpc_out "${GENERATED_PROTOBUF_PATH}" -I "${PROTO_PATH}"
         --plugin=protoc-gen-grpc="/usr/local/bin/grpc_cpp_plugin"
         "${HELLO_PROTO}"
    DEPENDS "${HELLO_PROTO}"
)

# Generated include directory
include_directories("${GENERATED_PROTOBUF_PATH}")

foreach(_target
  async_hello_server async_hello_client)
  add_executable(${_target} "${_target}.cc"
    ${HELLO_PROTO_SRC}
    ${HELLO_GRPC_SRC})
  target_link_libraries(${_target}
    ${CMAKE_THREAD_LIBS_INIT}
    protobuf
    grpc++_unsecure
    grpc)
endforeach()
