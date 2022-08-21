#!/bin/bash

source build_dir.sh

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
echo "Leaving SuperBuild directory"




cd ..
./runme.sh bundle
