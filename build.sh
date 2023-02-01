#!/bin/sh

set -xe

CFLAGS="-Wall -Wextra"
INCLUDES=`pkg-config --cflags sdl2`
LIBS=`pkg-config --libs sdl2`

mkdir build
cc $CFLAGS $INCLUDES -o ./build/colliding_blocks main.c $LIBS