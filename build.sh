#!/bin/sh

set -xe

CFLAGS="-Wall -Wextra -O3"
INCLUDES=`pkg-config --cflags sdl2`
LIBS="`pkg-config --libs sdl2` -lm"

if [ ! -d build ]; then
    mkdir build
fi
cc $CFLAGS $INCLUDES -o ./build/colliding_blocks main.c $LIBS