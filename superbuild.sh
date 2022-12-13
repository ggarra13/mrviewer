#!/bin/bash

source build_dir.sh

if [[ $KERNEL == 'Darwin' ]]; then
    brew upgrade
    brew install pkg-config ragel freetype glib cairo meson ninja
fi

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

./runme_bundle.sh

