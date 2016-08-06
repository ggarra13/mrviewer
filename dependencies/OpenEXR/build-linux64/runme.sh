#!/bin/sh

install=/usr/local

cmake .. -DCMAKE_BUILD_TYPE=Release -DILMBASE_PACKAGE_PREFIX=$install

export PATH="$install/lib:$PATH"

make
