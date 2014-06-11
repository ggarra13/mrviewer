#!/bin/bash

valgrind -v --trace-children=yes --track-origins=yes --show-reachable=no --leak-check=full $*


