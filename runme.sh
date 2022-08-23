#!/bin/bash

source build_dir.sh

if [ $KERNEL == 'Windows' ]; then
    if [ $CMAKE_BUILD_ARCH == 64 ]; then
	source vc14_win64_vars
    else
	source vc14_win32_vars
    fi
fi

export BOOST_ROOT=$PWD/SuperBuild/BUILD/$BUILD/tmp/BOOST-prefix/src/BOOST
echo "BOOST_ROOT=$BOOST_ROOT"

cd libAMF
./runme.sh
cd ..

cd libACESclip
./runme.sh
cd ..

export prefix=$PWD/install-$BUILD/
export LD_FLAGS="-Wl,--copy-dt-needed-entries"

echo "PREFIX=$prefix"
mk --prefix=$prefix -DFLTK_DIR=$PWD/SuperBuild/BUILD/$BUILD/Release/tmp/FLTK-prefix/src/FLTK-build/ $@
