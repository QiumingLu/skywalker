SOURCES=machine/master_machine.cc machine/membership_machine.cc machine/state_machine.cc network/messager.cc network/network.cc paxos/acceptor.cc paxos/checkpoint_manager.cc paxos/config.cc paxos/counter.cc paxos/group.cc paxos/instance.cc paxos/learner.cc paxos/node_impl.cc paxos/node_util.cc paxos/propose_queue.cc paxos/proposer.cc proto/paxos.pb.cc storage/db.cc util/crc32c.cc util/logging.cc util/mutex.cc util/options.cc util/runloop.cc util/status.cc util/thread.cc util/timerlist.cc 
CC=cc
CXX=c++
PLATFORM_LDFLAGS=-L/usr/local/lib -L/usr/lib
PLATFORM_LIBS=-lpthread -lprotobuf -lleveldb -lvoyager_rpc -lvoyager_core -lvoyager_port -lvoyager_util
PLATFORM_CCFLAGS=
PLATFORM_CXXFLAGS=-std=c++11 -Wall -Wextra -Wno-unused-parameter -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings -march=native -pthread -I/usr/local/include -I/usr/include
PLATFORM_SHARED_EXT=dylib
PLATFORM_SHARED_LDFLAGS=-dynamiclib -install_name /Users/mirantslu/Development/skywalker/
PLATFORM_SHARED_CFLAGS=-fPIC
