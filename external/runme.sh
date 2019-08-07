#/usr/bin/bash --norc

KERNEL=`uname`
RELEASE=`uname -r`
installdir=$PWD/../install-$KERNEL-$RELEASE
echo "INSTALLDIR = " $installdir
if [[ $KERNEL == Linux ]]; then
    copy='sudo cp'
    finaldir=/usr/local
else
    copy='cp'
    finaldir=/D/code/lib/vc14_Windows_$RELEASE
fi

if [[ $RELEASE == 3* ]]; then
    export CC=gcc-4.8
    export CXX=g++-4.8
    export CFLAGS=
    export CXXFLAGS=-std=c++11
fi

export PATH=$installdir/bin:$PATH
#export LD_LIBRARY_PATH=$installdir/lib:$LD_LIBRARY_PATH

../mk --installdir=$installdir -j 1 && echo "Install in /usr/local.  Needs root permissions" && $copy -r $installdir/* $finaldir
