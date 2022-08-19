#!/bin/bash

source build_dir.sh

if [ -e SuperBuild/BUILD/$BUILD ]; then
    rm -rf SuperBuild/BUILD/$BUILD
fi

if [ -e BUILD/$BUILD ]; then
    rm -rf BUILD/$BUILD
fi

cd SuperBuild
echo "Entering SuperBuild directory"
./runme.sh

echo "Leaving SuperBuild directory"
cd ..
./runme.sh bundle
