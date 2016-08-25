#!/bin/bash --norc

./b2 -j 8 toolset=msvc-12.0 --build-type=minimal --without-python variant=release link=shared threading=multi runtime-link=shared
