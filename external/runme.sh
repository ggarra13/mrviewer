#/usr/bin/bash --norc

install=$PWD/../install

RELEASE=`uname -r`

if [[ $RELEASE == 3* ]]; then
    export CC=gcc-4.8
    export CXX=g++-4.8
    export CFLAGS=
    export CXXFLAGS=-std=c++11
fi

export PATH=$install-$RELEASE/bin:$PATH
export LD_LIBRARY_PATH=$install-$RELEASE/lib:$LD_LIBRARY_PATH

../mk -v --installdir=$install-$RELEASE
sudo ../mk --installdir=/usr/local
