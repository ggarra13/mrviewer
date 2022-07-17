#/usr/bin/bash --norc

KERNEL=`uname`
RELEASE=`uname -r`
installdir=$PWD/../install-$KERNEL-$RELEASE
echo "INSTALLDIR = " $installdir
if [[ $KERNEL == Linux ]]; then
    copy='sudo cp'
    finaldir=/usr/local
elif [[ $KERNEL == Darwin ]]; then
    copy='sudo cp'
    finaldir=/usr/local
else
    copy='cp'
    finaldir=/D/code/lib/vc14_Windows_$RELEASE
    export CXXFLAGS=-DOCIO_USE_BOOST_PTR=1
fi


export LD_FLAGS="-Wl,--copy-dt-needed-entries"
export PATH=$installdir/bin:$PATH
export PKG_CONFIG_PATH=$installdir/lib/pkgconfig
export LD_LIBRARY_PATH=$installdir/lib:$LD_LIBRARY_PATH

../mk  --installdir=$installdir -j 1

if [[ $? == 0 ]]; then
    $copy -r $installdir/bin/* $finaldir/bin
    $copy -r $installdir/lib/* $finaldir/lib
    $copy -r $installdir/include/* $finaldir/include
    $copy -r $installdir/share/* $finaldir/share
    $copy -r $installdir/etc/* $finaldir/etc
    $copy -r $installdir/doc/* $finaldir/doc
fi
