#!/bin/bash

rm -rf Release/rails/assets2d/tmp/*

ver=$1
zip -9r mrViewer-$ver-Windows-x86.zip Release/ -x "Release/tmp/*" -x '*/*/#*#' -x '*/*/*~' -x "*.log"
