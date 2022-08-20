#/usr/bin/bash --norc

source ../build_dir.sh

installdir=$PWD/../install-$BUILD
echo "INSTALLDIR = " $installdir

../mk --installdir=$installdir --prefix=$installdir install
