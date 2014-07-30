#!/bin/bash

DJNI=`pwd`/dalvikhook/jni
LIB=`pwd`/examples/dynsec/jni
BASE=`pwd`

echo "compiling $DJNI..."
cd  $DJNI
ndk-build clean
ndk-build
echo "compiling lib $LIB"
cd $LIB
ndk-build clean
ndk-build
cd $BASE
adb push examples/dynsec/libs/armeabi/libdynsec.so /data/local/tmp

