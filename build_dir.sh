#!/bin/bash

#
# This file is to be sourced to set the BUILD/OS-RELEASE directory
#

#
# Turn on exit on error
#
set -o pipefail -e


#
# Determine CPU architecture
#
export KERNEL=`uname -s`

if [[ $KERNEL == MINGW* || $KERNEL == MSYS* ]]; then
    KERNEL=Windows
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

if [ $KERNEL == 'Windows' ]; then
    arch=`which cl.exe`
    if [[ $arch == *x86/cl* ]]; then
	export CMAKE_BUILD_ARCH=32
    fi
fi

export CMAKE_BUILD_TYPE="Release"
for i in $@; do
    case $i in
	debug)
	    export CMAKE_BUILD_TYPE="Debug"
	    break
	    ;;
    esac
done


export BUILD=$KERNEL-$CMAKE_BUILD_ARCH/$CMAKE_BUILD_TYPE
echo "BUILD DIRECTORY=$BUILD"
