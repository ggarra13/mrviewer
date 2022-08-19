#!/bin/bash

source build_dir.sh

export prefix=$PWD/install-$BUILD/
export LD_FLAGS="-Wl,--copy-dt-needed-entries"

mk --prefix=$prefix -DFLTK_DIR=$PWD/SuperBuild/BUILD/$BUILD/Release/tmp/FLTK-prefix/src/FLTK-build/ $@
