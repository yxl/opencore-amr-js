#!/bin/bash

# generate configure file
pushd .
cd ..
libtoolize --force
aclocal
automake --force-missing --add-missing
autoconf -i
popd

# build with emscripten to get llvm bitcode.
pushd .
rm -rf build
mkdir -p build
cd build
emconfigure ../../configure --prefix=`pwd`
emmake make install
cp lib/libopencore-amrnb.so ../amrnb.bc
popd

