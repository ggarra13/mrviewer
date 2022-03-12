#!/bin/bash

if [[ $BASH_SOURCE != $0 ]]; then
    dir=$BASH_SOURCE
else
    dir=$0
fi


symlink=`readlink ${dir}`

if [ "$symlink" != "" ]; then
    dir="$symlink"
    dir=${dir%/*}
else
# Find directory where mrViewer bash script resides
    dir=${dir%/*}
fi

# If running from current directory, get full path
if [ "$dir" == '.' ]; then
    dir=$PWD
fi




#
# Platform specific directory for mrViewer
#
dir=${dir%/*}

#
# Root directory where mrViewer is installed
#
root=${dir%/*/*}

#
# Add mrViewer's lib directory first to LD_LIBRARY_PATH
#
export LD_LIBRARY_PATH="${dir}/lib:/usr/local/lib:/lib/x86_64-linux-gnu:/usr/lib/x86_64-linux-gnu:x${LD_LIBRARY_PATH}"
export CTL_MODULE_PATH="${dir}/ctl:${CTL_MODULE_PATH}"

# Uncomment this to always use English in mrViewer, regardless of locale
# export LANG=C

# Comment this line if libfontconfig is configured properly
# export FONTCONFIG_PATH=/etc/fonts

# This is to avoid underruns in audio
export PULSE_LATENCY_MSEC=60

params=""

for param in "$@"
do
  params="${params} \"${param}\""
done

#
# Start up mrViewer
#
sh -c "${dir}/bin/mrViewer $params"
