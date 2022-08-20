#/usr/bin/bash --norc

source ../build_dir.sh

if [[ $KERNEL == 'Darwin' ]]; then
    brew install glib
    brew install glib-utils
    brew install meson
fi

installdir=$PWD/../install-$BUILD
echo "INSTALLDIR = " $installdir

if [[ $KERNEL == "Windows" ]]; then
    export BOOST_ROOT="/D/code/lib/boost_1_73_0"
    export FFMPEG_ROOT="/D/abs/local${CMAKE_BUILD_ARCH}/bin-video/ffmpegSHARED/"
fi

export LD_FLAGS="-Wl,--copy-dt-needed-entries"
export PATH=$installdir/bin:$PATH
export PKG_CONFIG_PATH=$installdir/lib/pkgconfig:$installdir/lib64/pkgconfig:$installdir/share/pkgconfig:
export LD_LIBRARY_PATH=$installdir/lib64:$installdir/lib:$LD_LIBRARY_PATH

../mk --installdir=$installdir --prefix=$installdir $@
