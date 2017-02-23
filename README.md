# Skywalker
A paxos library which can run on Linux, Mac OS X and FreeBSD, etc.

编译环境：
（1）Linux 2.6.8，GCC 4.8
（2）Max OSX 10.8，Clang 3.3

编译方法：

(1) LevelDB编译(https://github.com/google/leveldb/blob/master/README.md) 
进入third_party/leveldb目录 
执行 make
执行 sudo cp -rf out-shared/libleveldb.* /usr/local/lib/ 
执行 sudo cp -rf include/leveldb /usr/local/include

(2) Protobuf编译(https://github.com/google/protobuf/blob/master/src/README.md) 
进入third_party/protobuf目录 
执行 ./autogen.sh   
执行 ./configure
执行 make && sudo make install


(3) Voyager编译(https://github.com/QiumingLu/voyager/blob/master/README.md) 
进入third_party/voyager目录  
执行./build.sh。
进入./build/release目录，执行sudo make install

(4) Skywalker编译
1、通过CMake编译安装，和Voyager相同。
2、通过Makefile编译安装，和LevelDB相同。
