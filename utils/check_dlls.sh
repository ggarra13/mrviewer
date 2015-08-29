#!/bin/bash

echo "32 bits"
echo "-------"
cd ../BUILD/Windows-6.1-32

for i in Release/bin/*; do 
echo -n $i
dumpbin -headers $i | grep machine
done


echo
echo "64 bits"
echo "-------"

cd ../../BUILD/Windows-6.1-64

for i in Release/bin/*; do 
echo -n $i
dumpbin -headers $i | grep machine
done
