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
# Variables used to connect to the postgreSQL database
#
export MRV_HOST=127.0.0.1
export MRV_USER=mrviewer
export MRV_PASSWORD=mrviewer
export MRV_PORT=5432
export MRV_DATABASE=assets2d_production
export MRV_DATABASE_DRIVER=none  # postgresql

#
# Add mrViewer's lib directory first to LD_LIBRARY_PATH
#
magick_version=6.7.4
export LD_LIBRARY_PATH="${dir}/lib:${LD_LIBRARY_PATH}"
export CTL_MODULE_PATH="${dir}/ctl:${CTL_MODULE_PATH}"
export MAGICK_CODER_MODULE_PATH="${dir}/lib/ImageMagick-${magick_version}/modules-Q32/coders"

# Uncomment this to always use English in mrViewer, regardless of locale
# export LANG=C



params=""

for param in "$@"
do
  params="${params} \"${param}\""
done

#
# Start up mrViewer
#
sh -c "${dir}/bin/mrViewer $params"
