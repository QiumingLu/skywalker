set(PROTOC ${PROJECT_SOURCE_DIR}/third_party/protobuf/bin/protoc)
set(PROTOC_C_OUT_FLAG --cpp_out)
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR})
message(STATUS "PROTOC = ${PROTOC}")

file(GLOB PROTP_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/*.proto)

foreach(PROTO_FILE ${PROTP_INPUT})
  get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
  set(CUR_PROTO_GEN
    ${PROTO_GEN_DIR}/${PROTO_NAME}.pb.h
    ${PROTO_GEN_DIR}/${PROTO_NAME}.pb.cc
    )
  set(PROTO_GEN
    ${PROTO_GEN}
    ${CUR_PROTO_GEN}
    )
  message(STATUS "CUR_PROTO_GEN = ${CUR_PROTO_GEN}")

  add_custom_command(
    OUTPUT ${CUR_PROTO_GEN}
    COMMAND ${PROTOC} 
    ARGS ${PROTOC_C_OUT_FLAG} ${PROTO_GEN_DIR} ${PROTO_FILE} -I${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${PROTO_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endforeach()

include_directories(${PROJECT_BINARY_DIR})
set_source_files_properties(${PROTO_GEN} COMPILE_FLAGS "-Wno-conversion")

file(GLOB PROTO_GEN_CC ${PROTO_GEN}/*.cc)
file(GLOB PROTO_GEN_H ${PROTO_GEN}/*.h)

add_library(skywalker_proto ${PROTO_GEN})

set (paxos_SRCS
  acceptor.cc
  checkpoint_manager.cc
  config.cc
  counter.cc
  group.cc
  instance.cc
  learner.cc
  node_impl.cc
  nodeinfo.cc
  proposer.cc
  runloop.cc
  transfer.cc
  state_machine.cc
  state_machine_impl.cc
  network/messager.cc
  network/network.cc
  storage/db.cc
  )

add_library(skywalker_paxos ${paxos_SRCS})

target_link_libraries(skywalker_paxos skywalker_proto voyager_http protobuf leveldb)

install(TARGETS voyager_paxos DESTINATION lib)

set(HEADERS
  options.h
  nodeinfo.h
  node.h
  )

install(FILES ${HEADERS} DESTINATION include/voyager/paxos)

if (NOT CMAKE_BUILD_NO_EXAMPLES)
    add_subdirectory(tests)
endif()
