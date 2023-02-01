#!/bin/sh

set -xe

CFLAGS="-Wall -Wextra"
INCLUDES=`pkg-config --cflags sdl2`
LIBS=`pkg-config --libs sdl2`

cc $CFLAGS $INCLUDES -o ./colliding_blocks main.c $LIBS