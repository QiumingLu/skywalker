FILE(GLOB proto_files ${CMAKE_CURRENT_SOURCE_DIR}/paxos/*.proto)  
FOREACH(proto_file ${proto_files})
    STRING(REGEX REPLACE "[^/]proto" "" proto_file_name ${proto_file})
    LIST(APPEND proto_SRCS "${proto_file_name}.pb.cc")
	ADD_CUSTOM_COMMAND(
		OUTPUT "${proto_file_name}.pb.h" "${proto_file_name}.pb.cc"
		COMMAND protoc --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/paxos
                       --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/paxos/ ${proto_file} 
		DEPENDS ${proto_file}
	)
ENDFOREACH(proto_file)
include_directories(./paxos)

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
