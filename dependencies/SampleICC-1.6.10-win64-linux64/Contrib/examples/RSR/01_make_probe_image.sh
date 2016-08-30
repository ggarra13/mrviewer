#!/bin/sh -f

create_probe
creation_status=$?
if [ $creation_status != 0 ] ; then
    echo "error : $0:t : nonzero status ($creation_status) returned from create_probe"
    exit -1
fi

images_OK=1
if [ ! -e /var/tmp/32bpcProbe.tiff ] ; then
    echo "error : $0:t : failed to produce file /var/tmp/32bpcProbe.tiff"
    images_OK=0
fi
if [ ! -e /var/tmp/8bpcProbe.tiff ] ; then
    echo "error : $0:t : probe failed to produce file /var/tmp/8bpcProbe.tiff"
    images_OK=0
fi
if [ ! $images_OK == 1 ] ; then
    exit -1
fi

echo "Created:"
echo "  /var/tmp/32bpcProbe.tiff"
echo "  /var/tmp/8bpcProbe.tiff"
echo "Next, you should:"
echo " (0) turn one of the above into a 10-bit log Cineon/DPX file, in a simple"
echo "     linear scaling operation, using something like Photoshop, or After"
echo "     Effects, or Nuke, or whatever"
echo " (1) load the Cineon/DPX file into the tool that uses a cinePlugIn, e.g."
echo "     Shake with cineShake, and feed that into the cinePlugIn node, setting"
echo "     all the plugIn parameters just as you would for production use"
echo " (2) With the Cineon/DPX image now effectively capturing the RSR 'look',"
echo "     grab a copy of this with /Applications/Utilities/Grab.app's "
echo "     'Capture Selection' tool and save this into a new TIFF file."
echo " (3) Then run the next tool in the chain, 02_make_input_profile_from_probe.sh,"
echo "     passing it as arguments the pathname to the TIFF file you just saved,"
echo "     and the pathname of the input profile which you are trying to create."
exit 0
