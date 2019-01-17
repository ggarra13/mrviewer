#!/usr/bin/env bash

export FFMPEG_ROOT=/usr/local/sanitize/
export CTL_ROOT=/usr/local/sanitize/
export OPENEXR_ROOT=/usr/local/sanitize/

export LD_LIBRARY_PATH=/usr/local/sanitize/lib:$LD_LIBRARY_PATH
export TSAN_OPTIONS="second_deadlock_stack=1 ${TSAN_OPTIONS}"
