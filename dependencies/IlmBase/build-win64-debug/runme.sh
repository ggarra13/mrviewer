#!/bin/sh

install=/f/code/lib/Windows_64/Debug

cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$install -G 'NMake Makefiles'

export PATH="$install/lib:$PATH"

nmake
