This directory contains the OTIO<->reel adapter for OpenTimelineIO.
To use it, you need to set the environment variable:

OTIO_PLUGIN_MANIFEST_PATH=/dir/to/adapter/mrviewer.plugin_manifest.json

If you are using bash, you can just source the env.sh file here.  For Windows, you will need to set the variable in the Systems->Advanced Settings->Environment Variables or in a bat file to run from the console.
You will also need ffprobe (ffprobe.exe) to be in your PATH to process movie files.

Note that the current Windows version of OTIO has a bug in splitting the OTIO_PLUGIN_MANIFEST_PATH on colons instead of on semicolons.  The following file:

C:/Users/Gonzalo/AppData/Local/Programs/Python/Python37/Lib/site-packages/opentimelineio/plugins/manifest.py

needs to have line 259, changed from:
	for json_path in _local_manifest_path.split(':'):
to:
	for json_path in _local_manifest_path.split(os.pathsep):

You can do the above if you install the beta of opentimelineio, with:
    pip install opentimelineio --pre
