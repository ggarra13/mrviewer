#!/bin/sh

install=/usr/local/

cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$install 

export PATH="$install/lib:$PATH"

make
