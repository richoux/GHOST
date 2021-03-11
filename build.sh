#!/bin/bash

OS="$(uname)"
RELEASE="release"
RELEASEDBGINFO="release_with_debug_info"
DEBUG="debug"
OSX="_osx"
BACKPWD="$PWD"
EXPERIMENTAL=""
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
    echo "$0: usage: build.sh [release|rel_dbg_info|debug|clean|doc|tests] [EXP]"
    exit 1
}

function release()
{
    mkdir -p $RELEASE
    cd $RELEASE
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=$CXX $EXPERIMENTAL ..
    make
    sudo make install
}

function debug()
{
    mkdir -p $DEBUG
    cd $DEBUG
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=$CXX $EXPERIMENTAL ..
    make
    sudo make install
}

function rel_dbg_info()
{
    mkdir -p $RELEASEDBGINFO
    cd $RELEASEDBGINFO
    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_COMPILER=$CXX $EXPERIMENTAL ..
    make
    sudo make install    
}

function clean()
{
    if [ -d "$RELEASE" ]; then 
				cd $RELEASE
				make clean
				sudo rm -fr *
				cd ..
    fi
    if [ -d "$RELEASEDBGINFO" ]; then 
				cd $RELEASEDBGINFO
				make clean
				sudo rm -fr *
				cd ..
    fi
    if [ -d "$DEBUG" ]; then 
				cd $DEBUG
				make clean
				sudo rm -fr *
				cd ..
    fi
    if [ -d "build" ]; then 
				cd build
				make clean
				sudo rm -fr *
				cd ..
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

if [ $# -eq 2 ]; then
    if [ "$2" == "EXP" ]; then
				EXPERIMENTAL="-DGHOST_EXPERIMENTAL=ON"
    fi
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

