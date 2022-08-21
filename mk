#!/bin/bash

#
# Determine CPU architecture
#
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

CMAKE_OPTS=${CMAKE_OPTS=""}

export CMAKE_NATIVE_ARCH=32
export CMAKE_BUILD_TYPE=Release
export CMAKE_PROCS=4
if [[ $KERNEL == Darwin* ]]; then
    CMAKE_PROCS=8
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


export CMAKE_BUILD_ARCH=$CMAKE_NATIVE_ARCH

OS=$KERNEL-$RELEASE

usage()
{
    cat <<EOF
$0

$0 [options] [--] [make options]

Wrapper around CMake to easily create out of source builds
(ie. compilations where everything goes into a directory).

For this platform (default settings):
  BUILD/$OS-$CMAKE_BUILD_ARCH/$CMAKE_BUILD_TYPE

It must be run from a directory with a valid CMakeLists.txt
file outside of the build tree.

Options:

  -q         quiet build (default)
  -v         verbose build
  -j N       number of cpus to use (default: ${CMAKE_PROCS=8})
  -G <name>  use a different cmake generator (default: Ninja)

  debug|release|reldebug|small
  debug    - build a debug build
  release  - build a release build (default)
  reldebug - build a release build with debug information
  small    - build a small release build

  both|32|64
	     Builds both 32/64 bit versions, 32-bit only,
	     64-bit only (default: $CMAKE_BUILD_ARCH)

  cache    - Cleans all CMakeCache.txt files

  swig     - Cleans all swig files

  clean    - Cleans BUILD/$OS-$CMAKE_BUILD_ARCH/$CMAKE_BUILD_TYPE

  cmake    - Runs cmake only.

  compile  - Runs compile (Ninja, make, nmake, etc) only

EOF
    exit 1
}

clean_cache()
{
    if [ $CMAKE_BUILD_ARCH == 'Both' ]; then
	CMAKE_BUILD_ARCH=32
	clean_cache
	CMAKE_BUILD_ARCH=64
	clean_cache
	CMAKE_BUILD_ARCH=Both
	return
    fi

    builddir=BUILD/$OS-$CMAKE_BUILD_ARCH/$CMAKE_BUILD_TYPE
    if [ ! -d $builddir ]; then
	return
    fi
    echo
    echo "Removing old cmake caches $builddir..."
    echo
    # Remove cache files
    find $builddir -name CMakeCache.txt -exec rm {} \;
    # Remove makefiles
    find $builddir -name Makefile -exec rm {} \;
}


clean_swig()
{
    if [ $CMAKE_BUILD_ARCH == 'Both' ]; then
	CMAKE_BUILD_ARCH=32
	clean_swig
	CMAKE_BUILD_ARCH=64
	clean_swig
	CMAKE_BUILD_ARCH=Both
	return
    fi

    # Remove swig wrappers
    builddir=BUILD/$OS-$CMAKE_BUILD_ARCH/$CMAKE_BUILD_TYPE
    echo
    echo "Removing old swig wrappers $builddir..."
    echo
    find $builddir -name '*_wrap.*' -exec rm {} \;
}

#
# Parse command-line options
#
clean=0
cache=0


if [[ $OS == Windows* ]]; then
    #cmake_generator=Ninja
    cmake_generator="NMake Makefiles"
    win32cl=`which cl`
    if [[ $win32cl != *64* ]]; then
	CMAKE_BUILD_ARCH=32
    fi
else
    cmake_generator="Unix Makefiles"
fi


opts=''
RUN_CMAKE=1
RUN_MAKE=1
for i in $@; do
    case $i in
	64)
	    shift
	    CMAKE_BUILD_ARCH=64
	    ;;
	32)
	    shift
	    CMAKE_BUILD_ARCH=32
	    ;;
	both)
	    shift
	    CMAKE_BUILD_ARCH='Both'
	    ;;
	cache)
	    shift
	    cache=1
	    ;;
	clean)
	    if [ -r CMakeLists.txt ]; then
		shift
		if [ -d BUILD ]; then
		    clean=1
		fi
	    else
		break
	    fi
	    ;;
	-DCMAKE_INSTALL_PREFIX=*|--installdir=*)
	    shift
	    installdir="${i#*=}"
	    ;;
	-DCMAKE_PREFIX_PATH=*|--prefix=*)
	    shift
	    prefix="${i#*=}"
	    ;;
	compile)
	    shift
	    RUN_CMAKE=0
	    ;;
	cmake)
	    shift
	    RUN_MAKE=0
	    ;;
	debug)
	    shift
	    export CMAKE_BUILD_TYPE=Debug
	    ;;
	release)
	    shift
	    export CMAKE_BUILD_TYPE=Release
	    ;;
	reldebug)
	    shift
	    export CMAKE_BUILD_TYPE=RelWithDebInfo
	    ;;
	small)
	    shift
	    export CMAKE_BUILD_TYPE=MinSizeRel
	    ;;
	-DFLTK_DIR=*)
	    shift
	    fltk_dir="-DFLTK_DIR=${i#*=}"
	    ;;
	swig)
	    shift
	    clean_swig
	    ;;
	-h)
	    shift
	    usage
	    ;;
	--help)
	    shift
	    usage
	    ;;
	-j)
	    shift
	    if [ $# == 0 ]; then
		echo "Missing parameter for -j!"
		usage
	    fi
	    CMAKE_PROCS=$1
	    shift
	    ;;
	-q)
	    shift
	    opts=""
	    ;;
	-v)
	    shift
	    opts="-v"
	    ;;
	-G)
	    shift
	    cmake_generator=$1
	    shift
	    ;;
	--)
	    shift
	    break
	    ;;
    esac
done


#
# Simple function to run a command and print it out
#
run_cmd()
{
    echo
    echo "> $*"
    echo
    eval command $*
}

#
# Function to just run make on a dir with a Makefile
#
run_make()
{
    if [ $RUN_MAKE == 0 ]; then
	return
    fi

    make_cmd="make -j ${CMAKE_PROCS}"
    if [[ $cmake_generator == NMake* ]]; then
	make_cmd='nmake'
    fi

    if [ -f "Makefile" ]; then
	cmd="${make_cmd} $@"
    else
	cmd="ninja -j ${CMAKE_PROCS} $@"
    fi

    run_cmd $cmd
    status=$?
    if [ $status != 0 ]; then
	cd ../../../..
	exit $status
    fi
    cd ../../../..
}


#
# Function to run cmake and then run make on generated Makefile
#
run_cmake()
{

    builddir=$PWD/BUILD/$OS-$CMAKE_BUILD_ARCH/$CMAKE_BUILD_TYPE

    if [[ $installdir == "" ]]; then
	installdir=/usr/local
	if [[ $OS == Darwin* ]]; then
	    installdir=/usr/local
	fi

	if [[ $OS == Windows* ]]; then
	    installdir="D:/code/lib/vc14_Windows_${CMAKE_BUILD_ARCH}"
	    if [[ $CMAKE_BUILD_TYPE == Debug ]]; then
		installdir="${installdir}_debug"
	    fi
	fi
    fi


    if [ ! -d $installdir ]; then
	cmd="mkdir -p $installdir"
	run_cmd $cmd
    fi

    echo "Buildir ${builddir}"
    if [ ! -d $builddir/tmp ]; then
	cmd="mkdir -p $builddir $builddir/bin $builddir/lib $builddir/tmp"
	run_cmd $cmd
    fi

    cmd="cd $builddir/tmp"
    run_cmd $cmd

    if [ -r Makefile ]; then
	run_make $opts $@
	return
    fi


    pwd=$PWD

    if [[ $cmake_generator == *Makefile* ]]; then
	cmake_opts="-DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE"
    else
	cmake_opts="-DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE"
    fi

    cmd="cmake ../../../.. $fltk_dir -DCMAKE_PREFIX_PATH=$prefix -DCMAKE_INSTALL_PREFIX=$installdir -DEXECUTABLE_OUTPUT_PATH=$builddir/bin -DLIBRARY_OUTPUT_PATH=$builddir/lib -DCMAKE_LIBRARY_PATH=$builddir/lib -DCMAKE_NATIVE_ARCH=$CMAKE_NATIVE_ARCH -DCMAKE_BUILD_ARCH=$CMAKE_BUILD_ARCH ${cmake_opts} -G '${cmake_generator}'"


    if [ $RUN_CMAKE == 1 ]; then
	run_cmd  $cmd

	status=$?
	if [ $status != 0 ]; then
	    cd ../../../..
	    exit $status
	fi
    fi


    run_make $opts $@
}

#
# Function used to clean the build directory and exit
#
run_clean()
{
    if [ $CMAKE_BUILD_ARCH == "Both" ]; then
	if [ "$OS_64_BITS" == "1" ]; then
	    CMAKE_BUILD_ARCH=64
	    run_clean
	fi
	if [ "$OS_32_BITS" == "1" ]; then
	    CMAKE_BUILD_ARCH=32
	    run_clean
	fi
	CMAKE_BUILD_ARCH=Both
	return
    fi

    builddir=BUILD/$OS-$CMAKE_BUILD_ARCH/$CMAKE_BUILD_TYPE/tmp
    if [ -d $builddir ]; then
	if [ -e ninja.build ]; then
	    run_make clean
	fi
	if [ -e Makefile ]; then
	    run_make clean
	fi
	rm -rf $builddir/*
	echo "-------------------------------"
	echo "Cleaned $builddir"
	echo "-------------------------------"
	clean_cache
    fi
}


#
# Main routine
#
if [ $clean == 1 ]; then
    run_clean
    exit 0
elif [ $cache == 1 ]; then
    clean_cache
fi

if [ -r Makefile ]; then
    run_make $opts $@
else

    if [ ! -r CMakeLists.txt ]; then
	echo "No Makefile or CMakeLists.txt in current directory!"
	exit 1
    fi

    if [ ! -d BUILD ]; then
	mkdir BUILD
    fi

    if [ $CMAKE_BUILD_ARCH == 'Both' ]; then
	if [ "$OS_32_BITS" == "1" ]; then
	    export CMAKE_BUILD_ARCH=32
	    if [ "$OS_64_BITS" == "1" ]; then
		export LDFLAGS=-m32 ${LDFLAGS}
		export CXXFLAGS=-m32 ${CXXFLAGS}
		export CFLAGS=-m32 ${CFLAGS}
	    fi
	    run_cmake $opts $@
	fi

	if [ "$OS_64_BITS" == "1" ]; then
	    export CMAKE_BUILD_ARCH=64
	    export LDFLAGS=${LDFLAGS}
	    export CXXFLAGS=${CXXFLAGS}
	    export CFLAGS=${CFLAGS}
	    run_cmake $opts $@
	fi
    else
	run_cmake $opts $@
    fi
fi
