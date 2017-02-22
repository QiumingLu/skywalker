CXX = g++
CXXFLAGS = -std=c++11 -g -Wall -Wextra \
		   -Wno-unused-parameter -Woverloaded-virtual \
		   -Wpointer-arith -Wshadow -Wwrite-strings \
		   -march=native -pthread \
		   -I/usr/local/include -I/usr/include \
		   -I../../include \
		   -I../../third_party/leveldb/include \

LDFLAGS = -L/usr/local/lib -I/usr/lib \
		  -L../../out-static \
		  -L../../third_party/leveldb/out-static \
		  -lskywalker -lleveldb -lprotobuf \
		  -lvoyager_rpc -lvoyager_core -lvoyager_port -lvoyager_util \
		  -lpthread

PROTOC = protoc

PROTOS_PATH = .

vpath %.proto $(PROTOS_PATH)

all: journey_client journey_server

journey_client: journey.pb.o journey_client.o
	$(CXX) $^ $(LDFLAGS) -o $@

journey_server: journey_db.o journey_db_machine.o \
                journey.pb.o journey_service_impl.o journey_server.o
	$(CXX) $^ $(LDFLAGS) -o $@

%.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<

.PHONY: clean
clean:
	-rm -f *.o *.pb.cc *.pb.h journey_client journey_server
