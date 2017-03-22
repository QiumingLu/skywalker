CXX=c++
PLATFORM_LDFLAGS=-L/usr/local/lib -L/usr/lib -L../../out-static
PLATFORM_LIBS=-lpthread -lskywalker -lprotobuf -lleveldb -lvoyager_rpc -lvoyager_core -lvoyager_port -lvoyager_util
PLATFORM_CXXFLAGS=-std=c++11 -Wall -Wextra -Wno-unused-parameter -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings -march=native -pthread -I/usr/local/include -I/usr/include -I../../include
