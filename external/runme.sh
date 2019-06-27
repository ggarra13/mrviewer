#/usr/bin/bash --norc


RELEASE=`uname -r`
installdir=$PWD/../install-$RELEASE

if [[ $RELEASE == 3* ]]; then
    export CC=gcc-4.8
    export CXX=g++-4.8
    export CFLAGS=
    export CXXFLAGS=-std=c++11
fi

export PATH=$installdir/bin:$PATH
export LD_LIBRARY_PATH=$installdir/lib:$LD_LIBRARY_PATH

../mk --installdir=$installdir -v && sudo ../mk --installdir=/usr/local
