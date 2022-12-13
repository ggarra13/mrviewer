#!/bin/bash


. build_dir.sh


usage()
{
    cat <<EOF
$0

$0 [options] [--] [make options]

Wrapper around CMake to easily create out of source builds
(ie. compilations where everything goes into a directory).

For this platform (default settings):
  BUILD/$BUILD_DIR

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

  clean    - Cleans BUILD/$BUILD

  cmake    - Runs cmake only.

  compile  - Runs compile (Ninja, make, nmake, etc) only

EOF
    exit 1
}

clean_cache()
{
    echo
    echo "Removing old cmake caches BUILD/$BUILD..."
    echo
    # Remove cache files
    find BUILD/$BUILD -name CMakeCache.txt -exec rm {} \;
    # Remove makefiles
    find BUILD/$bUILD -name Makefile -exec rm {} \;
}

#
# Parse command-line options
#
clean=0
cache=0

cmake_generator=Ninja

if [[ $OS == Windows* ]]; then
    win32cl=`which cl`
    if [[ $win32cl != *64* ]]; then
	CMAKE_BUILD_ARCH=32
    fi
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

    cmd="cmake --build . -j ${CMAKE_PROCS} $@ --config $CMAKE_BUILD_TYPE -v"
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

    builddir=$PWD/BUILD/$BUILD

    if [[ $installdir == "" ]]; then
	installdir=/usr/local
	if [[ $OS == Darwin* ]]; then
	    installdir=/usr/local
	fi

	if [[ $OS == Windows* ]]; then
	    installdir="E:/code/lib/vc14_Windows_${CMAKE_BUILD_ARCH}"
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


    pwd=$PWD

    cmd="cmake ../../../.. $fltk_dir -D CMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_PREFIX_PATH=$prefix -DCMAKE_INSTALL_PREFIX=$installdir -DCMAKE_BUILD_ARCH=$CMAKE_BUILD_ARCH -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -G '${cmake_generator}'"


    if [ $RUN_CMAKE == 1 ]; then
	run_cmd  $cmd
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

    builddir=BUILD/$BUILD/tmp
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
