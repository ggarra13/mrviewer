#!/bin/bash
#
# This script is to be sourced in a current shell environment, like:
#
# $ source env.sh
#
# It will set the PATH variable to point to the local ffprobe and
# the OTIO_PLUGIN_MANIFEST_PATH to the local otio<->reel adapter.
#

if [[ $BASH_SOURCE == $0 ]]; then
    echo "Please source this script from a bash shell.  Do not run it."
    exit 1
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export PATH=$DIR/../../bin:$PATH
export OTIO_PLUGIN_MANIFEST_PATH=$DIR/mrviewer.plugin_manifest.json
