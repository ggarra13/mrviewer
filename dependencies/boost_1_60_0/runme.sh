#!/bin/bash --norc

# ./b2 -j 8 --build-type=minimal address-model=64 --without-python variant=release link=shared threading=multi runtime-link=shared

./b2 -j 8 --without-python address-model=64 variant=release
