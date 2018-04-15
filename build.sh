#!/bin/bash

OS="$(uname)"
RELEASE="release"
DEBUG="debug"
OSX="_osx"
BACKPWD="$PWD"

if [ "$OS" == "Darwin" ]; then
    RELEASE="$RELEASE$OSX"
    DEBUG="$DEBUG$OSX"
fi

function usage()
{
    echo "$0: usage: build.sh [release|debug|clean|doc|tests]"
    exit 1
}

function release()
{
    mkdir -p $RELEASE
    cd $RELEASE
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make
    sudo make install
}

function debug()
{
    mkdir -p $DEBUG
    cd $DEBUG
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    make
    sudo make install
}

function clean()
{
    if [ -d "$RELEASE" ]; then 
	cd $RELEASE
	make clean
    fi
    if [ -d "$DEBUG" ]; then 
	cd $DEBUG
	make clean
    fi
    if [ -d "build" ]; then 
	cd build
	make clean
    fi
}

function doc()
{
    doxygen doc/Doxyfile
}

function tests()
{
    cd test
    mkdir -p build
    cd build
    cmake ..
    make
}

if [ $# -gt 1 ]; then
    usage
fi

if [ $# -eq 0 ]; then
    release
    cd $BACKPWD
    exit 0
fi

if [ "$1" == "release" ]; then
    release
    cd $BACKPWD
    exit 0
elif [ "$1" == "debug" ]; then
    debug
    cd $BACKPWD
    exit 0
elif [ "$1" == "clean" ]; then
    clean
    cd $BACKPWD
    exit 0
elif [ "$1" == "doc" ]; then
    doc
    cd $BACKPWD
    exit 0
elif [ "$1" == "tests" ]; then
    tests
    cd $BACKPWD
    exit 0
else
    usage
fi

