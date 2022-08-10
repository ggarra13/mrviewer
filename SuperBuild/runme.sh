#/usr/bin/bash --norc

KERNEL=`uname -s`

if [[ $KERNEL == CYGWIN* ]]; then
    KERNEL=Windows
    RELEASE=(`cmd /c 'ver'`)
    RELEASE=${RELEASE[3]%.[0-9]*}
elif [[ $KERNEL == MINGW* ]]; then
    RELEASE=(`cmd /c 'ver'`)
    #RELEASE=${RELEASE[3]%.[0-9]*}
    RELEASE=${RELEASE[3]/]/}
    KERNEL=Windows
else
    RELEASE=`uname -r`
fi


if [[ "$RELEASE" == "" ]]; then
    $RELEASE=`cmd.exe /c 'ver'`
    echo $RELEASE
    echo "Could not determine OS version"
    exit 1
fi

export CMAKE_NATIVE_ARCH=32
export CMAKE_BUILD_TYPE=Release
export CMAKE_PROCS=4

export OS_32_BITS=1
export OS_64_BITS=

export fltk_dir=""

if [ $KERNEL == 'Windows' ]; then
    arch=`wmic OS get OSArchitecture`
    if [[ $arch == *64* ]]; then
	CMAKE_NATIVE_ARCH=64
	export OS_64_BITS=1
    fi
else
    arch=`uname -a`
fi


if [[ $arch == *64* ]]; then
    CMAKE_NATIVE_ARCH=64
    export OS_64_BITS=1
fi

export CMAKE_BUILD_ARCH=$CMAKE_NATIVE_ARCH

if [[ $KERNEL == 'Windows' ]]; then
    win32cl=`which cl`
    if [[ $win32cl != *64* ]]; then
	CMAKE_BUILD_ARCH=32
    fi
fi

BUILD=$KERNEL-$RELEASE-$CMAKE_BUILD_ARCH

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

../mk $@ --installdir=$installdir --prefix=$installdir -j 1
