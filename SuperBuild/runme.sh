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
else
    if [[ $arch == *ia64* ]]; then
	CMAKE_NATIVE_ARCH=64
	export OS_64_BITS=1
	unset  OS_32_BITS
    fi
fi

BUILD=$KERNEL-$RELEASE-$CMAKE_NATIVE_ARCH

installdir=$PWD/../install-$BUILD
echo "INSTALLDIR = " $installdir


export LD_FLAGS="-Wl,--copy-dt-needed-entries"
export PATH=$installdir/bin:$PATH
export PKG_CONFIG_PATH=$installdir/lib/pkgconfig:$installdir/lib64/pkgconfig:$installdir/share/pkgconfig:
export LD_LIBRARY_PATH=$installdir/lib:$LD_LIBRARY_PATH

../mk --installdir=$installdir --prefix=$installdir -j 1
