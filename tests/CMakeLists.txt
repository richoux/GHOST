cmake_minimum_required (VERSION 3.1)
project (ghost_tests)

# require a C++14-capable compiler
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX1Y)
if(COMPILER_SUPPORTS_CXX14)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
elseif(COMPILER_SUPPORTS_CXX1Y)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

## These two lines are the reason why we need CMake version 3.1.0+
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# add the targets
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

################################
# Unit Tests
################################
# Add tests cpp file
add_executable( test_domain src/test_domain.cpp )
add_executable( test_variable src/test_variable.cpp )
add_executable( test_constraint src/test_constraint.cpp )

target_link_libraries(test_domain gtest gtest_main ghost Threads::Threads)
target_link_libraries(test_variable gtest gtest_main ghost Threads::Threads)
target_link_libraries(test_constraint gtest gtest_main ghost Threads::Threads)

enable_testing()
add_test( test_domain test_domain )
add_test( test_variable test_variable )
add_test( test_constraint test_constraint )
