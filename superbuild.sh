#!/bin/bash

if [ -e SuperBuild/BUILD ]; then
    rm -rf SuperBuild/BUILD
fi

if [ -e BUILD ]; then
    rm -rf BUILD
fi

cd SuperBuild
echo "Entering SuperBuild directory"
./runme.sh

echo "Leading SuperBuild directory"
cd ..
./runme.sh bundle
