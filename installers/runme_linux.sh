#!/bin/bash --norc


tar -czhf mrViewer-x64.tar.gz --exclude='tmp*' -C../BUILD/Linux-`uname -r`-64/ Release
# tar -czhf mrViewer-i86.tar.gz --exclude='tmp*' -C../BUILD/Linux-`uname -r`-32/ Release
