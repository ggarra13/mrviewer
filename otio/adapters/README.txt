This directory contains the OTIO<->reel adapter for OpenTimelineIO.
To use it, you need to set the environment variable:

OTIO_PLUGIN_MANIFEST_PATH=/dir/to/adapter/mrviewer.plugin_manifest.json

If you are using bash, you can just source the env.sh file here.  For Windows, you will need to set the variable in the Systems->Advanced Settings->Environment Variables or in a bat file to run from the console.

You will also need ffprobe (ffprobe.exe) to be in your PATH to process movie files.
