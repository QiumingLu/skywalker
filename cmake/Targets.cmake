macro(skywalker_set_skywalker_link)
  set(Skywalker_LINK skywalker)
endmacro()

file(GLOB_RECURSE proto_files proto/*.proto)
skywalker_protobuf_generate_cpp(${proto_gen_folder} proto_srcs proto_hdrs ${proto_files})
set_source_files_properties(${proto_hdrs} ${proto_srcs} PROPERTIES COMPILE_FLAGS "-Wno-conversion -Wno-shorten-64-to-32 -Wno-deprecated-declarations -fPIC")

file(GLOB_RECURSE util_srcs ${CMAKE_CURRENT_SOURCE_DIR}/util/*.cc)
file(GLOB_RECURSE storage_srcs ${CMAKE_CURRENT_SOURCE_DIR}/storage/*.cc)
file(GLOB_RECURSE network_srcs ${CMAKE_CURRENT_SOURCE_DIR}/network/*.cc)
file(GLOB_RECURSE paxos_srcs ${CMAKE_CURRENT_SOURCE_DIR}/paxos/*.cc)
file(GLOB_RECURSE machine_srcs ${CMAKE_CURRENT_SOURCE_DIR}/machine/*.cc)

set(skywalker_srcs
  ${proto_srcs}
  ${util_srcs}
  ${storage_srcs}
  ${network_srcs}
  ${paxos_srcs}
  ${machine_srcs}
 )
