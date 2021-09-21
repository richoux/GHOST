#!/bin/bash

OS="$(uname)"
RELEASE="release"
RELEASEDBGINFO="release_with_debug_info"
DEBUG="debug"
OSX="_osx"
BACKPWD="$PWD"
CXX="g++"

RED='\033[0;31m'
GREEN='\033[0;32m'
ORANGE='\033[0;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

RED='\033[0;31m'
GREEN='\033[0;32m'
ORANGE='\033[0;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

if [ "$OS" == "Darwin" ]; then
    RELEASE="$RELEASE$OSX"
    RELEASEDBGINFO="$RELEASEDBGINFO$OSX"
    DEBUG="$DEBUG$OSX"
    CXX="clang++"
fi

function usage()
{
    echo "$0: usage: build.sh [release|rel_dbg_info|debug|debug_no_asan|clean|doc|tests|tutorial]"
    exit 1
}

function release()
{
    mkdir -p build/$RELEASE
    cd build/$RELEASE
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=$CXX ../..
    make
    sudo make install
}

function debug()
{
    mkdir -p build/$DEBUG
    cd build/$DEBUG
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=$CXX ../..
    make
    sudo make install
}

function debug_no_asan()
{
    mkdir -p build/$DEBUG
    cd build/$DEBUG
    cmake -DCMAKE_BUILD_TYPE=Debug -DNO_ASAN=ON -DCMAKE_CXX_COMPILER=$CXX ../..
    make
    sudo make install
}

function rel_dbg_info()
{
    mkdir -p build/$RELEASEDBGINFO
    cd build/$RELEASEDBGINFO
    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_COMPILER=$CXX ../..
    make
    sudo make install    
}

function clean()
{
    if [ -d "build/$RELEASE" ]; then 
				cd build/$RELEASE
				make clean
				sudo rm -fr *
				cd $BACKPWD
    fi
    if [ -d "$build/RELEASEDBGINFO" ]; then 
				cd build/$RELEASEDBGINFO
				make clean
				sudo rm -fr *
				cd $BACKPWD
    fi
    if [ -d "$build/DEBUG" ]; then 
				cd build/$DEBUG
				make clean
				sudo rm -fr *
				cd $BACKPWD
    fi
}

function doc()
{
    doxygen doc/Doxyfile
}

function tests()
{
    cd tests
    mkdir -p build
    cd build
    cmake ..
    make
}

function tutorial()
{
    cd tutorial
    mkdir -p build
    cd build
    cmake ..
    make
}

function first_compile()
{
    if [ "$OS" == "Linux" ]; then
				echo -e "\n\n${RED}>>> If you compile ${GREEN}GHOST${RED} for the ${CYAN}first time${RED}, you probably need to run the following command: ${ORANGE}sudo ldconfig${NC}"
    fi

    if [ "$OS" == "Darwin" ]; then
				echo -e "\n\n${RED}>>> If you compile ${GREEN}GHOST${RED} for the ${CYAN}first time${RED}, you probably need to run the following command: ${ORANGE}sudo update_dyld_shared_cache${NC}"
    fi
}

if [ $# -gt 2 ]; then
    usage
fi

if [ $# -eq 0 ]; then
    release
    first_compile
    cd $BACKPWD
    exit 0
fi

if [ "$1" == "release" ]; then
    release
    first_compile
    cd $BACKPWD
    exit 0
elif [ "$1" == "rel_dbg_info" ]; then
    rel_dbg_info
    first_compile
    cd $BACKPWD
    exit 0
elif [ "$1" == "debug" ]; then
    debug
    cd $BACKPWD
    exit 0
elif [ "$1" == "debug_no_asan" ]; then
    debug_no_asan
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
elif [ "$1" == "tutorial" ]; then
    tutorial
    cd $BACKPWD
    exit 0
else
    usage
fi

