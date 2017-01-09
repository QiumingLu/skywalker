PREFIX = ../..
SKYWALKER_INCLUDE=$(PREFIX)/include
SKYWALKER_LIBRARY=$(PREFIX)/out-static
VOYAGER_INCLUDE=$(PREFIX)/third_party/voyager/include
VOYAGER_LIBRARY=$(PREFIX)/third_party/voyager/lib
LEVELDB_INCLUDE=$(PREFIX)/third_party/leveldb/include
LEVELDB_LIBRARY=$(PREFIX)/third_party/leveldb/lib
PROTOBUF_INCLUDE=$(PREFIX)/third_party/protobuf/include
PROTOBUF_LIBRARY=$(PREFIX)/third_party/protobuf/lib

CXX = g++
CXXFLAGS = -std=c++11 -g -pthread \
		   -I$(SKYWALKER_INCLUDE) -I$(VOYAGER_INCLUDE) \
		   -I$(LEVELDB_INCLUDE) -I$(PROTOBUF_INCLUDE)

LDFLAGS = -L$(SKYWALKER_LIBRARY) -L$(VOYAGER_LIBRARY) \
		  -L$(LEVELDB_LIBRARY) -L$(PROTOBUF_LIBRARY) \
		  -lskywalker -lvoyager_core -lvoyager_port -lvoyager_util \
		  -lleveldb -lprotobuf -lpthread

PROTOC = $(PREFIX)/third_party/protobuf/bin/protoc

PROTOS_PATH = .

vpath %.proto $(PROTOS_PATH)

all: journey_client journey_server

journey_client: rpc.pb.o rpc_codec.o rpc_channel.o \
                journey.pb.o journey_client.o
	$(CXX) $^ $(LDFLAGS) -o $@

journey_server: rpc.pb.o rpc_codec.o rpc_server.o \
                journey.pb.o journey_server.o
	$(CXX) $^ $(LDFLAGS) -o $@

%.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<

.PHONY: clean
clean:
	-rm -f *.o *.pb.cc *.pb.h journey_client journey_server
