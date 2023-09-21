#!/bin/bash

OS="$(uname)"
RELEASE="release"
RELEASEDBGINFO="release_with_debug_info"
DEBUG="debug"
ANDROID="android"
OSX="_osx"
BACKPWD="$PWD"
CXX="g++"

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
    echo "$0: usage: build.sh [release|rel_dbg_info|debug|debug_no_asan|android|clean|doc|tests|tutorial]"
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

function android()
{
		if [[ -z "${ANDROID_NDK}" ]]; then
				echo "The environment variable ANDROID_NDK must be defined with the path of your NDK directory."
				echo "For instance: export ANDROID_NDK=/path/Android/sdk/ndk/25.2.9519653"
				exit 1
		fi
		
    mkdir -p build/${ANDROID}_arm64
    cd build/${ANDROID}_arm64
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=31 -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a -DCMAKE_ANDROID_NDK="${ANDROID_NDK}" -DCMAKE_C_COMPILER="/usr/bin/aarch64-linux-gnu-gcc-11" -DCMAKE_CXX_COMPILER="/usr/bin/aarch64-linux-gnu-g++-11" -DCMAKE_CROSSCOMPILING=TRUE ../..
    make

		cd ../..
    mkdir -p build/${ANDROID}_armelf
    cd build/${ANDROID}_armelf
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=31 -DCMAKE_ANDROID_ARCH_ABI=armeabi-v7a -DCMAKE_ANDROID_NDK="${ANDROID_NDK}" -DCMAKE_C_COMPILER="/usr/bin/aarch64-linux-gnu-gcc-11" -DCMAKE_CXX_COMPILER="/usr/bin/aarch64-linux-gnu-g++-11" -DCMAKE_CROSSCOMPILING=TRUE ../..
		make

		cd ../..
    mkdir -p build/${ANDROID}_x86
    cd build/${ANDROID}_x86
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=31 -DCMAKE_ANDROID_ARCH_ABI=x86 -DCMAKE_ANDROID_NDK="${ANDROID_NDK}" -DCMAKE_C_COMPILER="/usr/bin/aarch64-linux-gnu-gcc-11" -DCMAKE_CXX_COMPILER="/usr/bin/aarch64-linux-gnu-g++-11" -DCMAKE_CROSSCOMPILING=TRUE ../..
		make

		cd ../..
    mkdir -p build/${ANDROID}_x86_64
    cd build/${ANDROID}_x86_64
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=31 -DCMAKE_ANDROID_ARCH_ABI=x86_64 -DCMAKE_ANDROID_NDK="${ANDROID_NDK}" -DCMAKE_C_COMPILER="/usr/bin/aarch64-linux-gnu-gcc-11" -DCMAKE_CXX_COMPILER="/usr/bin/aarch64-linux-gnu-g++-11" -DCMAKE_CROSSCOMPILING=TRUE ../..
    make
}

function clean()
{
    if [ -d "build/$RELEASE" ]; then 
				cd build/$RELEASE
				make clean
				sudo rm -fr *
				cd $BACKPWD
    fi
    if [ -d "build/$RELEASEDBGINFO" ]; then 
				cd build/$RELEASEDBGINFO
				make clean
				sudo rm -fr *
				cd $BACKPWD
    fi
    if [ -d "build/$DEBUG" ]; then 
				cd build/$DEBUG
				make clean
				sudo rm -fr *
				cd $BACKPWD
    fi
    if [ -d "build/${ANDROID}_arm64" ]; then 
				cd build/${ANDROID}_arm64
				make clean
				sudo rm -fr *
				cd $BACKPWD
    fi
    if [ -d "build/${ANDROID}_armelf" ]; then 
				cd build/${ANDROID}_armelf
				make clean
				sudo rm -fr *
				cd $BACKPWD
    fi
    if [ -d "build/${ANDROID}_x86" ]; then 
				cd build/${ANDROID}_x86
				make clean
				sudo rm -fr *
				cd $BACKPWD
    fi
    if [ -d "build/${ANDROID}_x86_64" ]; then 
				cd build/${ANDROID}_x86_64
				make clean
				sudo rm -fr *
				cd $BACKPWD
    fi
		if [ -d "tests/build" ]; then 
				cd tests/build/
				make clean
				sudo rm -fr *
				cd $BACKPWD
    fi
		if [ -d "tutorial/wiki/build" ]; then 
				cd tutorial/wiki/build/
				make clean
				sudo rm -fr *
				cd $BACKPWD
    fi
		if [ -d "tutorial/video/build" ]; then 
				cd tutorial/video/build/
				make clean
				sudo rm -fr *
				cd $BACKPWD
    fi
}

function doc()
{
		if [[ -z $(git status -s) ]]
		then
				doxygen doc/Doxyfile
				DATETODAY=$(date +%Y-%m-%d)
				mkdir -p "../doc_temp_copy_$DATETODAY"
				cp -r doc/html/* "../doc_temp_copy_$DATETODAY"
				CURRENTBRANCH=$(git rev-parse --abbrev-ref HEAD)
				git checkout gh-pages
				CURRENTFOLDER=$(basename "$PWD")
				cd "../doc_temp_copy_$DATETODAY"
				yes | cp -fr * "../$CURRENTFOLDER"
				cd "../$CURRENTFOLDER"
				git add -A
				git commit -am "Documentation from $DATETODAY"
				git push
				git checkout "$CURRENTBRANCH"
				rm -fr "../doc_temp_copy_$DATETODAY"
		else
				echo -e "${RED}>>> You must commit your changes before running this command.${NC}\n"
				git status
		fi		
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
		cd wiki
    mkdir -p build
    cd build
    cmake ..
    make
		cd ../../video
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
elif [ "$1" == "android" ]; then
    android
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

