#!/bin/sh

# PATHS must be absolute for cmake to work
install=/D/code/lib/Windows_64/

cmake .. -DCMAKE_BUILD_TYPE=Release -DILMBASE_PACKAGE_PREFIX=$install -DCMAKE_INSTALL_PREFIX=$install -DZLIB_LIBRARY=D:/code/applications/mrViewer/dependencies/zlib-1.2.8-win64/zlib.lib -DZLIB_INCLUDE_DIR=../../zlib-1.2.8-win64/ -G 'NMake Makefiles'

export PATH="$install/lib:$PATH"

nmake
