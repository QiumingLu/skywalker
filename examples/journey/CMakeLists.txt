include_directories(${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate_cpp(proto_srcs proto_hdrs journey.proto)
set_source_files_properties(${proto_srcs} ${proto_hdrs} PROPERTIES COMPILE_FLAGS "-Wno-conversion -Wno-shorten-64-to-32 -Wno-deprecated-declarations -fPIC")

add_executable(journey_client journey_client.cc ${proto_srcs})
target_link_libraries(journey_client ${Skywalker_LINKER_LIBS} ${Skywalker_LINK})

add_executable(journey_server journey_server.cc
                              journey_service_impl.cc
                              journey_db_machine.cc journey_db.cc
                              ${proto_srcs})
target_link_libraries(journey_server ${Skywalker_LINKER_LIBS} ${Skywalker_LINK})
