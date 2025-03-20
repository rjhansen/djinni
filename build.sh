#!/bin/sh

rm -rf build
mkdir build
cd build || exit
cmake -S .. -B . -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_CXX_FLAGS=-DUSE_BOUNDS_CHECKING \
    -D CMAKE_INSTALL_PREFIX=${HOME}
make -j8
make install
cd ..
rm -rf build
