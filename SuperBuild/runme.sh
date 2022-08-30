#!/bin/bash

source ../build_dir.sh

installdir=$PWD/../install-$BUILD
echo "INSTALLDIR = " $installdir



if [[ $KERNEL == "Windows" ]]; then
    export FFMPEG_ROOT="/D/abs/local${CMAKE_BUILD_ARCH}/bin-video/ffmpegSHARED/"
fi

#export LD_FLAGS="-Wl,--copy-dt-needed-entries"
export PATH=$installdir/bin:$PATH
export PKG_CONFIG_PATH=$installdir/lib/pkgconfig:$installdir/lib64/pkgconfig:$installdir/share/pkgconfig:$PKG_CONFIG_PATH
export LD_LIBRARY_PATH=$installdir/lib64:$installdir/lib:/usr/local/lib:$LD_LIBRARY_PATH

echo "PKG_CONFIG_PATH=$PKG_CONFIG_PATH"

../mk --installdir=$installdir --prefix=$installdir $@
