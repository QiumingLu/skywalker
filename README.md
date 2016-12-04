# SkyWalker
A paxos library which can run on Linux, Mac OS X and FreeBSD, etc.

编译环境：
（1）Linux 2.6.8，GCC 4.8
（2）Max OSX 10.8，Clang 3.3

编译方法：

(1) LevelDB编译
SkyWalker的编译流程是基于leveldb1.19版本，如果你自行下载了其他版本，可能有编译上的不同。

进入third_party/leveldb目录。 
执行 make，编译完成后会在当前目录生成out-static/libleveldb.a文件。 
执行 mkdir lib，建立一个lib目录，然后执行 cd lib，执行 ln -s ../out-static/libleveldb.a,  
libleveldb.a建立一个软链，SkyWalker通过lib这个目录来寻址静态库。 

(2) Protobuf编译
进入third_party/protobuf目录。 
执行 ./autogen.sh   
执行 ./configure CXXFLAGS=-fPIC --prefix=[当前目录绝对路径], 这一步CXXFLAGS和--prefix都必须设置对。 
执行 make && make install
编译完成后检查是否在当前目录成功生成bin,include,lib三个子目录。

(3) Voyager编译
进入third_party/voyager的根目录下。
执行 sh build.sh，然后执行 cp -r ../build/release-install/* ./ 即可,若不再需要Voyager编译生成的文件,可以执行 rm -rf ../build 删除

(4) SkyWalker编译
进入SkyWalker 主目录。执行 make 即可，生成目录out-static，里面包含libskywalker.a和一些可执行文件。
