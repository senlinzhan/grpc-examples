# Generated include directory
include_directories("${GENERATED_PROTOBUF_PATH}")
foreach(_target
  async_company_client)
  add_executable(${_target} "${_target}.cc"
    ${COMPANY_PROTO_SRC}
    ${COMPANY_GRPC_SRC})
  target_link_libraries(${_target}
    protobuf
    grpc++_unsecure
    ${CMAKE_THREAD_LIBS_INIT})
endforeach()
