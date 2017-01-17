# SkyWalker
A paxos library which can run on Linux, Mac OS X and FreeBSD, etc.

编译环境：
（1）Linux 2.6.8，GCC 4.8
（2）Max OSX 10.8，Clang 3.3

编译方法：

(1) LevelDB编译(https://github.com/google/leveldb/blob/master/README.md) 
进入third_party/leveldb目录 
执行 make

(2) Protobuf编译(https://github.com/google/protobuf/blob/master/src/README.md) 
进入third_party/protobuf目录 
执行 ./autogen.sh   
执行 ./configure --disable-shared
执行 make && sudo make install


(3) Voyager编译(https://github.com/QiumingLu/voyager/blob/master/README.md) 
进入third_party/voyager目录  
执行 sh build.sh。

(4) SkyWalker编译
进入SkyWalker 目录
执行 make
