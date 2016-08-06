#!/bin/sh

install=/f/code/lib/Windows_32/
zlib=/f/code/lib/zlib-1.2.8-win32/lib/zdll.lib



cmake .. -DCMAKE_BUILD_TYPE=Release -DILMBASE_PACKAGE_PREFIX=$install -DCMAKE_INSTALL_PREFIX=$install -DZLIB_LIBRARY=$zlib -DZLIB_INCLUDE_DIR=/f/code/lib/zlib-1.2.8-win32/include -G 'NMake Makefiles'

cp $zlib $install/lib

export PATH="$install/lib:$PATH"

nmake