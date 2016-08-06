#!/bin/sh

install=/f/code/lib/Windows_64/

cmake .. -DCMAKE_BUILD_TYPE=Release -DILMBASE_PACKAGE_PREFIX=$install -DCMAKE_INSTALL_PREFIX=/f/code/lib/Windows_64 -DZLIB_LIBRARY=/f/code/lib/zlib-1.2.8-win64/zlib.lib -DZLIB_INCLUDE_DIR=/f/code/lib/zlib-1.2.8-win64/ -G 'NMake Makefiles'

export PATH="$install/lib:$PATH"

nmake