cmake_minimum_required (VERSION 3.15)
project (ghost_tests)

set( CMAKE_VERBOSE_MAKEFILE on )

# require a C++20-capable compiler
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)
CHECK_CXX_COMPILER_FLAG("-std=c++2a" COMPILER_SUPPORTS_CXX2A)
CHECK_CXX_COMPILER_FLAG("/std:c++20" COMPILER_SUPPORTS_CXX20_WIN)
if(COMPILER_SUPPORTS_CXX20)
  set(CMAKE_CXX_FLAGS "-std=c++20")
elseif(COMPILER_SUPPORTS_CXX2A)
  set(CMAKE_CXX_FLAGS "-std=c++2A")
elseif(COMPILER_SUPPORTS_CXX20_WIN)
  set(CMAKE_CXX_FLAGS "/std:c++20")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++20 support. Please use a different C++ compiler.")
endif()

if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
	INCLUDE_DIRECTORIES("C:/Program Files (x86)/ghost/include")
	INCLUDE_DIRECTORIES("C:/Program Files (x86)/googletest-distribution/include/")
	link_directories("C:/Program Files (x86)/googletest-distribution/lib/")
	link_directories("C:/Program Files (x86)/ghost/lib")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()	

if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem\ /usr/local/include")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fsanitize=address,undefined,leak")
	set(CMAKE_EXE_FLAGS_DEBUG "-fsanitize=address,undefined,leak")
endif()

## These two lines are the reason why we need CMake version 3.1.0+
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# add the targets
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

################################
# Unit Tests
################################
# Add tests cpp file
add_executable( test_variable src/test_variable.cpp )

if(APPLE)
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		target_link_libraries(test_variable /usr/local/lib/libgtest.a /usr/local/lib/libghost_staticd.a Threads::Threads)
	else()
		target_link_libraries(test_variable /usr/local/lib/libgtest.a /usr/local/lib/libghost_static.a Threads::Threads)
	endif()
else()	
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		target_link_libraries(test_variable gtest ghostd Threads::Threads)
	else()
		target_link_libraries(test_variable gtest ghost Threads::Threads)
	endif()
endif()
	
enable_testing()
add_test( NAME Test_Variable COMMAND test_variable WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
