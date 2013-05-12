#!/bin/bash

rm -rf Release/rails/assets2d/tmp/*

ver=$1

tar -czhf mrViewer-$ver-Linux-x64.tar.gz Release/ --exclude=Release/tmp   --exclude-backups

