#!/bin/bash

source build_dir.sh

echo "INSTALL LIBRARIES in install-$BUILD"

if [ -d install-$BUILD ]; then
    echo "Removing install-$BUILD"
    rm -rf install-$BUILD/$CMAKE_BUILD_TYPE
    if [ $? != 0 ]; then
	exit $?
    fi
fi

if [ -d SuperBuild/BUILD/$BUILD ]; then
    echo "Removing SuperBuild/BUILD/$BUILD"
    rm -rf SuperBuild/BUILD/$BUILD
    if [ $? != 0 ]; then
	exit $?
    fi
fi

if [ -d BUILD/$BUILD ]; then
    echo "Removing BUILD/$BUILD"
    rm -rf BUILD/$BUILD
    if [ $? != 0 ]; then
	exit $?
    fi
fi


cd SuperBuild
echo "Entering SuperBuild directory"

./runme.sh $@

if [ $? != 0 ]; then
    exit $?
fi


echo "Leaving SuperBuild directory"
cd ..

release=''

if [[ $KERNEL == *Linux* ]]; then
    release=`lsb_release -d`
fi
if [[ $release == *Ubuntu* ]]; then
    ./runme.sh $@
    if [ $? != 0 ]; then
	exit $?
    fi

    ./utils/libs.rb
else
    ./runme.sh bundle $@
    if [ $? != 0 ]; then
	exit $?
    fi
fi
