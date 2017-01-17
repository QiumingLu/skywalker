# SkyWalker
A paxos library which can run on Linux, Mac OS X and FreeBSD, etc.

编译环境：
（1）Linux 2.6.8，GCC 4.8
（2）Max OSX 10.8，Clang 3.3

编译方法：

(1) LevelDB编译
进入third_party/leveldb目录。 
执行 make，编译完成后会在当前目录生成out-static/libleveldb.a文件。 
执行cp ./out-static/libleveldb.a /usr/local/lib, 将生成的libleveldb.a文件拷贝到/usr/local/lib中。
执行cp -r include/* /usr/local/include, 将leveldb所需要的头文件拷贝到/usr/local/include中。

(2) Protobuf编译
进入third_party/protobuf目录。 
执行 ./autogen.sh   
执行 ./configure --prefix=/usr/local。 
执行 make && make install


(3) Voyager编译
进入third_party/voyager的根目录下。
执行 sh build.sh。
执行 cp -r ./build/release-install/lib/* /usr/local/lib, 将生成的lib文件拷贝到/usr/local/lib中。
执行 cp -r ./build/release-install/include/* /usr/local/include, 将voyager所需要的头文件拷贝到/usr/local/include 中。

(4) SkyWalker编译
进入SkyWalker 主目录。执行 make 即可，生成目录out-static，里面包含libskywalker.a和一些可执行文件。
