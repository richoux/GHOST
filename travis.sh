#!/usr/bin/env sh

echo $CXX
$CXX --version

git clone https://github.com/google/googletest.git

# install GTest
cd googletest/googletest
mkdir build ; cd build
cmake ..
make
cp -r ../include/* ../../../test/include/
cp libg*a ../../../test/lib/

# install GMock
cd ../../googlemock
mkdir build ; cd build
cmake ..
make
cp -r ../include/* ../../../test/include/
cp libg*a ../../../test/lib/
cd ../../..  

# install GHOST
export LD_LIBRARY_PATH=/usr/local/lib/
mkdir build
cd build
cmake ..
make VERBOSE=1
sudo make install
cd ..

# - make CXX=g++-5
# - sudo make install

# make and run tests
make test
cd test/bin && for i in $(ls); do ./$i; done

# - valgrind --leak-check=full --show-reachable=yes bin/ghost --auto
# - valgrind --tool=cachegrind bin/ghost --auto
