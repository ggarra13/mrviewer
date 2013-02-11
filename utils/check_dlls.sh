#!/bin/bash

for i in Release/bin/*; do 
echo -n $i
dumpbin -headers $i | grep machine
done
