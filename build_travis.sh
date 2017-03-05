# Detect OS
if test -z "$TARGET_OS"; then
    TARGET_OS=`uname -s`
fi

if [ "$TARGET_OS" = "Darwin" ]; then
    INSTALL_PATH=/usr/local
else
    INSTALL_PATH=/usr
fi

CXX=g++
CXXFLAGS="-std=c++11 -pthread -I/usr/local/include -I/usr/include"
CXXOUTPUT="/tmp/skywalker_build_detect_platform-cxx.$$"

# Test whether Protobuf library is installed
$CXX $CXXFLAGS -x c++ - -o $CXXOUTPUT 2>/dev/null  <<EOF
    #include <google/protobuf/arena.h>
    int main() {}
EOF
  if [ "$?" != 0 ]; then
    wget https://github.com/google/protobuf/archive/v3.2.0.tar.gz
    tar zxvf v3.2.0.tar.gz
    cd protobuf-3.2.0
    ./autogen.sh
    ./configure --prefix=$INSTALL_PATH
    make
    sudo make install
    cd ..
    rm -rf protobuf-3.2.0
    rm -rf v3.2.0.tar.gz
  fi

$CXX $CXXFLAGS -x c++ - -o $CXXOUTPUT 2>/dev/null  <<EOF
    #include <leveldb/db.h>
    int main() {}
EOF
  if [ "$?" != 0 ]; then
    wget https://github.com/google/leveldb/archive/v1.20.tar.gz
    tar zxvf v1.20.tar.gz
    cd leveldb-1.20
    make
    sudo cp -rf out-shared/libleveldb.* $INSTALL_PATH/lib
    sudo cp -rf include/leveldb $INSTALL_PATH/include
    cd ..
    rm -rf leveldb-1.20
    rm -rf v1.20.tar.gz
  fi

$CXX $CXXFLAGS -x c++ -o $CXXOUTPUT 2>/dev/null  <<EOF
    #include <voyager/rpc/rpc_server.h>
    int main() { }
EOF
  if [ "$?" != 0 ]; then
    wget https://github.com/QiumingLu/voyager/archive/v1.6.tar.gz
    tar zxvf v1.6.tar.gz
    cd voyager-1.6
    cmake -DCMAKE_BUILD_TYPE=release \
          -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH \
          -DCMAKE_BUILD_NO_EXAMPLES=0 \
          -DCMAKE_BUILD_SHARED_LIBS=1 \
          .
    make
    sudo make install
    cd ..
    rm -rf voyager-1.6
    rm -rf v1.6.tar.gz
  fi

rm -f $CXXOUTPUT
