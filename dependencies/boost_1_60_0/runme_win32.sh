#!/bin/bash --norc

./b2 -j 8 toolset=msvc-12.0 --without-python address-model=32 variant=release

#./b2 -j 8 --without-python
