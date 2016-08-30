#!/bin/bash --norc

install=D:/code/lib/Windows_32/
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$install -G 'NMake Makefiles'


export PATH="$install/lib:$PATH"
nmake

cp ../zlib.h .
