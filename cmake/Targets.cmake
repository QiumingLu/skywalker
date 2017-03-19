macro(skywalker_set_skywalker_link)
  set(Skywalker_LINK skywalker)
endmacro()

file(GLOB proto_files proto/*.proto)
skywalker_protobuf_generate_cpp(${proto_gen_folder} proto_SRCS proto_HDRS ${proto_files})

file(GLOB util_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/util/*.cc)
file(GLOB storage_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/storage/*.cc)
file(GLOB network_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/network/*.cc)
file(GLOB paxos_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/paxos/*.cc)
file(GLOB machine_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/machine/*.cc)

set(skywalker_SRCS
  ${proto_SRCS}
  ${util_SRCS}
  ${storage_SRCS}
  ${network_SRCS}
  ${paxos_SRCS}
  ${machine_SRCS}
 )
