#!/bin/sh

rm -rf build
mkdir build
cd build || exit
if id -nG "$USER" | grep -qw "wheel"; then
    install_path=/usr/local
    cmake -S .. -B . -D CMAKE_BUILD_TYPE=Release \
        -D CMAKE_CXX_FLAGS=-DUSE_BOUNDS_CHECKING
else
    install_path=${HOME}
    cmake -S .. -B . -D CMAKE_BUILD_TYPE=Release \
        -D CMAKE_CXX_FLAGS=-DUSE_BOUNDS_CHECKING \
        -D CMAKE_INSTALL_PREFIX="${HOME}"
fi
make -j8
if id -nG "$USER" | grep -qw "wheel"; then
    sudo make install
else
    make install
fi

strip src/djinni_example
mv src/djinni_example ..
cp ../src/Dumas-1.set ..

cd ..
rm -rf build

echo
echo
echo "* * * * * * * * * *"
echo
echo "Djinni has been installed to ${install_path}/include"
echo
echo "To use Djinni in your own code:"
echo
echo "* tell your compiler to add ${install_path/include} to your include path"
echo "  (-I${install_path}/include works for most compilers)"
echo "* tell your compiler to optimize the code for performance"
echo "  (-O2 works for most)"
echo "* tell your compiler what to call the output (-o on most)"
echo "* tell your compiler to use the C++23 standard (-std=c++23 on most)"
echo "* add an '#include <djinni.h>' to your C++ sources"
echo
echo "For instance, to compile src/example.cc with your choice of the"
echo "Intel icpx, LLVM clang++, or GNU C++ compilers, you could enter:"
echo
echo "* icpx src/example.cc -std=c++23 -I${install_path}/include -O2 -o djinni_example"
echo "* clang++ src/example.cc -std=c++23 -I${install_path}/include -O2 -o djinni_example"
echo "* g++ src/example.cc -std=c++23 -I${install_path}/include -O2 -o djinni_example"
echo
echo "... and you're off to the races.  You can also type './djinni_example'"
echo "in this directory to see it in action, study the file 'Dumas-1.set' to"
echo "see the problem it's approximately solving, or look at 'src/example.cc'"
echo "to see the source code of the example."
echo
echo "Happy hacking!"
echo
echo "    -- Rob"
echo
