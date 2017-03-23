macro(skywalker_set_skywalker_link)
  set(Skywalker_LINK skywalker)
endmacro()

file(GLOB_RECURSE proto_files proto/*.proto)
skywalker_protobuf_generate_cpp(${proto_gen_folder} proto_SRCS proto_HDRS ${proto_files})

file(GLOB_RECURSE util_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/util/*.cc)
file(GLOB_RECURSE storage_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/storage/*.cc)
file(GLOB_RECURSE network_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/network/*.cc)
file(GLOB_RECURSE paxos_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/paxos/*.cc)
file(GLOB_RECURSE machine_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/machine/*.cc)

set(skywalker_SRCS
  ${proto_SRCS}
  ${util_SRCS}
  ${storage_SRCS}
  ${network_SRCS}
  ${paxos_SRCS}
  ${machine_SRCS}
 )
