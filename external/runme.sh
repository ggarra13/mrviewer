#/usr/bin/bash --norc

KERNEL=`uname`
RELEASE=`uname -r`
finaldir=/usr/local
installdir=$PWD/../install-$KERNEL-$RELEASE

if [[ $RELEASE == 3* ]]; then
    export CC=gcc-4.8
    export CXX=g++-4.8
    export CFLAGS=
    export CXXFLAGS=-std=c++11
fi

export PATH=$installdir/bin:$PATH
export LD_LIBRARY_PATH=$installdir/lib:$LD_LIBRARY_PATH

../mk --installdir=$installdir -v && echo "Install in /usr/local.  Needs root permissions" && sudo cp -r $installdir/* $finaldir
