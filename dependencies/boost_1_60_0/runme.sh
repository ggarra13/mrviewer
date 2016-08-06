#!/bin/bash --norc

./b2 --build-type=minimal --without-python variant=release link=shared threading=multi runtime-link=shared
