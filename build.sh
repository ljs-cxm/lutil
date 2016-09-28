#!/bin/bash

if [ -d build ]; then
    rm -rf build
fi

mkdir build

pushd build

cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
make

name=lutil
linux_lib=${name}.so
mac_lib=${name}.dylib

if [ -f $linux_lib ]; then
    cp $linux_lib ..
else
    cp $mac_lib ..
fi

popd
