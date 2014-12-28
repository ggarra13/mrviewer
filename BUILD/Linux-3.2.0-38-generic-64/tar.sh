#!/bin/bash

rm -rf Release/rails/assets2d/tmp/*

cp -f ../../HISTORY.txt Release
ver=$1
dir=mrViewer-v$ver-Linux-x64

function clean() {
    mv $dir Release
    exit 0
}


mv Release $dir

trap clean INT



tar -cjhf mrViewer-v$ver-Linux-x64.tar.bz2 $dir --exclude=$dir/tmp   --exclude-backups --exclude='*.fuse*'

clean

