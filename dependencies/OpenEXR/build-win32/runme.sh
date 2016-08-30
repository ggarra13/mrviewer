#!/bin/sh

install=/D/code/lib/Windows_32/
zlib=/D/code/lib/Windows_32/



cmake .. -DCMAKE_BUILD_TYPE=Release -DILMBASE_PACKAGE_PREFIX=$install -DCMAKE_INSTALL_PREFIX=$install -DZLIB_LIBRARY=$zlib/lib/zdll.lib -DZLIB_INCLUDE_DIR=$zlib/include -G 'NMake Makefiles'

cp $zlib $install/lib

export PATH="$install/lib:$PATH"

nmake
