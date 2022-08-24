#!/bin/bash

source build_dir.sh

if [ -d install-$BUILD ]; then
    echo "Removing install-$BUILD"
    rm -rf install-$BUILD
fi

if [ -d SuperBuild/BUILD/$BUILD ]; then
    echo "Removing SuperBuild/BUILD/$BUILD"
    rm -rf SuperBuild/BUILD/$BUILD
fi

if [ -d BUILD/$BUILD ]; then
    echo "Removing BUILD/$BUILD"
    rm -rf BUILD/$BUILD
fi


cd SuperBuild
echo "Entering SuperBuild directory"

./runme.sh

if [ $? != 0 ]; then
    exit $?
fi


echo "Leaving SuperBuild directory"
cd ..


release=`lsb_release -d`
if [[ $release == *Ubuntu* ]]; then
    ./runme.sh
    ./utils/libs.rb
else
    ./runme.sh bundle
fi
