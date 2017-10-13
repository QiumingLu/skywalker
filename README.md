
# Skywalker -- 分布式一致性协议Paxos的实现
## 简介
Skywalker是一个基于[Leslie Lamport](https://en.wikipedia.org/wiki/Leslie_Lamport)的[Paxos Made Simple](http://lamport.azurewebsites.net/pubs/paxos-simple.pdf)实现的C++ Paxos库，它的目标是使得单机服务能够很容易地扩展为分布式服务，并且具有强一致性和高度的容错能力。
<br/>
<br/>[![Build Status](https://travis-ci.org/QiumingLu/skywalker.svg?branch=master)](https://travis-ci.org/QiumingLu/skywalker)
<br/>
<br/>Author: Mirants Lu (mirantslu@gmail.com)
<br/>
<br/>**Skywalker主要包括以下几个部分：**
<br/>
<br/>1. **include/skywalker**：对外开放的接口文件；
<br/>2. **util**：基础库模块，包括互斥量、条件变量、线程、字符串处理、时间处理等基础工具；
<br/>3. **network**：网络传输模块，处理Skywalker的网络传输，在Voyager网络库的基础上搭建；
<br/>4. **storage**：日志存储模块，处理Skywalker的日志存储，在LevelDB数据库的基础上搭建；
<br/>5. **paxos**：Paxos协议实现模块，根据Leslie Lamport的Paxos Made Simple一文来实现；
<br/>6. **machine**：状态机模块，完成Skywalker的内部状态机的状态转移功能；
<br/>7. **proto**: Proto消息定义模块，定义Skywalker的消息协议，在Protobuf序列化库的基础上搭建；
<br/>8. **examples**：Skywalker的使用示例，目前在Skywalker的基础上实现了一个分布式KV数据库；
<br/>9. **third_party**: 第三方库，目前使用的第三方库有：LevelDB，Protobuf，Voyager；
<br/>10. **cmake**：CMake编译文件。


## 特性
* 基于Leslie Lamport的Paxos Made Simple来实现。
* 一个Node节点实例可以同时运行多个Paxos Group。
* Propose提议不阻塞用户线程，以异步架构实现，并且严格地按照用户提交顺序来进行提交，结果通过Callback的形式向用户返回提交结果。
* 一次Propose提议的完成，每台机器的性能消耗可以做到只有一次网络RTT，一次磁盘写入。
* 自动定时地发起学习操作，使得集群内的机器能够及时地保持状态对齐，可以正常地参与Paxos协议过程，降低集群停止正常工作的风险。
* 如果用户已经实现checkpoint接口，对于状态落后较大的节点，Skywalker会自动地通过checkpoint来进行学习，从而可以加快学习的速度。
* 内置Master选举功能。
* 内置成员变更管理配置功能。
* 基于Voyager来完成网络传输功能。
* 基于LevelDB来完成日志存储功能。
* 基于Protobuf来完成消息序列化功能。
* 接口简单方便使用，基本的操作为Propose。

## 局限
* 没有做特别的过载保护，可能会导致节点在高压的情况下不能自适应地运转。
* 没有实现实时监控功能，需要开发者自行开发。

## 性能

## 兼容性
Skywalker支持Linux，Mac OS X，FreeBSD等类Unix平台，不支持Windows平台，以下是一些曾测试的平台/编译器组合：
* Linux 4.4.0，GCC 5.4.0 
* macOS 10.12，Clang 3.6.0

## 编译依赖
* LevelDB  v1.3及以上版本
* Protobuf v3.0.0及以上版本
* Voyager  v1.0.0及以上版本

## 编译安装
(1) LevelDB编译安装(https://github.com/google/leveldb/blob/master/README.md) 
* 进入third_party/leveldb目录 
* 执行 make 
* 执行 sudo cp -rf out-shared/libleveldb.* /usr/local/lib/ 
* 执行 sudo cp -rf include/leveldb /usr/local/include

(2) Protobuf编译安装(https://github.com/google/protobuf/blob/master/src/README.md) 
* 进入third_party/protobuf目录 
* 执行 ./autogen.sh
* 执行 ./configure 
* 执行 make && sudo make install

(3) Voyager编译安装(https://github.com/QiumingLu/voyager/blob/master/README.md) 
* 进入third_party/voyager目录
* 执行./build.sh。 
* 进入./build/release目录，
* 执行sudo make install

(4) Skywalker编译安装
* 通过CMake编译安装，和Voyager相同。 
* 通过Makefile编译安装，和LevelDB相同。

## 使用示例

<br />1、简单地使用Skywalker的基本功能，可以参考examples/journey 。
<br />2、在Skywalker的基础上搭建分布式服务，可以参考https://github.com/QiumingLu/saber 。
