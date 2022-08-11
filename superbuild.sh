#!/bin/bash

cd SuperBuild
echo "Entering SuperBuild directory"
./runme.sh

echo "Leading SuperBuild directory"
cd ..
./runme.sh bundle
