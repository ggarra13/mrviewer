#!/bin/bash 


symlink=`readlink ${0}`

if [ "$symlink" != "" ]; then
    dir="$symlink"
    dir=${dir%/*}
else
# Find directory where mrViewer bash script resides
    dir=${0%/*}
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
export LD_LIBRARY_PATH="${dir}/lib:${LD_LIBRARY_PATH}"
export CTL_MODULE_PATH="${dir}/ctl:${CTL_MODULE_PATH}"

# Uncomment this to always use English in mrViewer, regardless of locale
# export LANG=C

# Uncomment this to use ImageMagick's PNG reader which works in a single
# thread and is slower, but may be more compatible and also allows all
# sort of gaps in the sequence.
#
# export DO_NOT_USE_FFMPEG_PNG=1

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
