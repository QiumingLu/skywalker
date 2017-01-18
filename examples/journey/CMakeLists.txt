include_directories(${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS journey.proto)

add_executable(journey_client journey_client.cc ${PROTO_SRCS})
target_link_libraries(journey_client skywalker)

add_executable(journey_server journey_server.cc journey_service_impl.cc 
                              journey_db_machine.cc journey_db.cc 
                              ${PROTO_SRCS})
target_link_libraries(journey_server skywalker ${SkyWalker_LINKER_LIBS})
