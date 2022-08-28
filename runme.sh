#!/bin/bash

source build_dir.sh

if [ $KERNEL == 'Windows' ]; then
    if [ $CMAKE_BUILD_ARCH == 64 ]; then
	source vc14_win64_vars
    else
	source vc14_win32_vars
    fi
fi

export prefix=$PWD/install-$BUILD/
export LD_FLAGS="-Wl,--copy-dt-needed-entries"
export FLTK_DIR=$PWD/SuperBuild/BUILD/$BUILD/tmp/FLTK-prefix/src/FLTK-build/

echo "PREFIX=$prefix"
mk --prefix=$prefix -DFLTK_DIR=$FLTK_DIR $@
