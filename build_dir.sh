#!/bin/bash

#
# This file is to be sourced to set the BUILD/OS-RELEASE directory
#

#
# Determine CPU architecture
#
export KERNEL=`uname -s`
export RELEASE=""

if [[ $KERNEL == MINGW* ]]; then
    RELEASE=(`cmd /c 'ver'`)
    #RELEASE=${RELEASE[3]%.[0-9]*}
    RELEASE=${RELEASE[3]/]/}
    KERNEL=Windows
else
    RELEASE=`uname -r`
fi

export OS_32_BITS=1
export OS_64_BITS=

export fltk_dir=""

if [ $KERNEL == 'Windows' ]; then
    arch=`wmic OS get OSArchitecture`
else
    arch=`uname -a`
fi

if [[ $arch == *64* ]]; then
    export CMAKE_NATIVE_ARCH=64
    export OS_64_BITS=1
fi

export CMAKE_BUILD_ARCH=$CMAKE_NATIVE_ARCH

arch=`which cl.exe`
echo $arch
if [[ $arch == *x86/cl* ]]; then
    export CMAKE_BUILD_ARCH=32
fi


export BUILD=$KERNEL-$RELEASE-$CMAKE_BUILD_ARCH
echo "BUILD DIRECTORY=$BUILD"
