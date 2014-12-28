#!/bin/bash

rm -rf Release/rails/assets2d/tmp/*


ver=$1
dir=mrViewer-v$ver-Windows-x32


function clean() {
    mv $dir Release
    exit 0
}

release.sh

mv Release $dir

trap clean INT


zip -9r mrViewer-v$ver-Windows-x32.zip $dir/ -x "$dir/tmp/*" -x "$dir/lib/*" -x '*/*/#*#' -x '*/*/*~' -x "*.log"

clean



#dist.sh $ver
