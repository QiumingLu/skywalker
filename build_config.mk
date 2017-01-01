SOURCES=machine/master_machine.cc machine/membership_machine.cc machine/state_machine.cc network/messager.cc network/network.cc paxos/acceptor.cc paxos/checkpoint_manager.cc paxos/config.cc paxos/counter.cc paxos/group.cc paxos/instance.cc paxos/learner.cc paxos/node_impl.cc paxos/node_util.cc paxos/paxos.pb.cc paxos/proposer.cc paxos/runloop.cc storage/db.cc util/crc32c.cc util/logging.cc util/mutex.cc util/options.cc util/status.cc util/thread.cc util/timerlist.cc 
CC=cc
CXX=g++
PLATFORM=OS_LINUX
PLATFORM_LDFLAGS=-pthread -L./third_party/voyager/lib -L./third_party/leveldb/lib -L./third_party/protobuf/lib
PLATFORM_LIBS= -lvoyager_core -lvoyager_port -lvoyager_util -lleveldb -lprotobuf
PLATFORM_CCFLAGS= -pthread -DOS_LINUX
PLATFORM_CXXFLAGS=-std=c++11 -Wall -Wextra -Wno-unused-parameter -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings -march=native -pthread -DOS_LINUX -I./third_party/voyager/include -I./third_party/leveldb/include -I./third_party/protobuf/include
