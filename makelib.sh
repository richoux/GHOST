#!/bin/bash

g++ -std=c++14 -fPIC -c src/domain.cpp -o obj/domain.o
g++ -std=c++14 -fPIC -c src/variable.cpp -o obj/variable.o
g++ -shared -o bin/libghost.so obj/domain.o obj/variable.o
