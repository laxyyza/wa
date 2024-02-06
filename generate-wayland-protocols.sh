#!/usr/bin/env bash

XDG_SHELL=wayland-protocols/xdg-shell.xml
XDG_DECORATION=wayland-protocols/xdg-decoration-unstable-v1.xml
INCLUDE_DIR=wa/include
SRC_DIR=wa/src

HEADER_CMD="wayland-scanner client-header"
CODE_CMD="wayland-scanner private-code"

function gen_headers()
{
    $HEADER_CMD $XDG_SHELL $INCLUDE_DIR/xdg-shell.h
    $HEADER_CMD $XDG_DECORATION $INCLUDE_DIR/xdg-decoration.h 
}

function gen_code()
{
    $CODE_CMD $XDG_SHELL $SRC_DIR/xdg-shell.c
    $CODE_CMD $XDG_DECORATION $SRC_DIR/xdg-decoration.c
}

gen_headers
gen_code
