#!/bin/bash

set -x

SYSTEM=`uname -a`
SOURCE_DIR=`pwd`
BUILD_DIR=${BUILD_DIR:-./build}
BUILD_TYPE=${BUILD_TYPE:-release}
INSTALL_DIR=${INSTALL_DIR:-/usr/local}
BUILD_NO_EXAMPLES=${BUILD_NO_EXAMPLES:-0}
BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS:-1}

mkdir -p $BUILD_DIR/$BUILD_TYPE \
  && cd $BUILD_DIR/$BUILD_TYPE \
  && cmake \
           -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
           -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
           -DCMAKE_BUILD_NO_EXAMPLES=$BUILD_NO_EXAMPLES \
           -DCMAKE_BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS} \
           $SOURCE_DIR \
  && make \
#  && sudo make install

