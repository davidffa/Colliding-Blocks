#!/bin/sh

set -xe

CFLAGS="-Wall -Wextra -O3"
INCLUDES="`pkg-config --cflags sdl2 sdl2_ttf`"
LIBS="`pkg-config --libs sdl2 sdl2_ttf` -lm"

if [ ! -d build ]; then
    mkdir build
fi
cc $CFLAGS $INCLUDES -o ./build/colliding_blocks main.c $LIBS