#!/bin/sh

grep -vE '^#:|POT-Creation-Date' $1
