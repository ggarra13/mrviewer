#!/bin/bash --norc


# Find directory where mrViewer bash script resides
dir=${0%/*}

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

#
# Add mrViewer's lib directory first to LD_LIBRARY_PATH
#
magick_version=6.7.4
export LD_LIBRARY_PATH="${dir}/lib:${LD_LIBRARY_PATH}"
export CTL_MODULE_PATH="${dir}/ctl:${dir}/ctl/odt/dcdm:${dir}/ctl/odt/hdr_pq:${dir}/ctl/odt/rgbMonitor:${dir}/ctl/odt/p3:${dir}/ctl/odt/rec2020:${dir}/ctl/odt/rec709:${dir}/ctl/rrt:${dir}/ctl/lmt:${dir}/ctl/idt:${dir}/ctl/ACESproxy:${dir}/ctl/ACEScg:${dir}/ctl/ACEScc:${CTL_MODULE_PATH}"
export MAGICK_CODER_MODULE_PATH="${dir}/lib/ImageMagick-${magick_version}/modules-Q32/coders"

#
# Start up mrViewer
#
"${dir}/bin/mrViewer" $*
