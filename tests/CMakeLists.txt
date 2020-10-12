cmake_minimum_required (VERSION 3.1)
project (ghost_tests)

set( CMAKE_VERBOSE_MAKEFILE on )

# require a C++14-capable compiler
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX1Y)
CHECK_CXX_COMPILER_FLAG("/std:c++14" COMPILER_SUPPORTS_CXX14_WIN)
if(COMPILER_SUPPORTS_CXX14)
  set(CMAKE_CXX_FLAGS "-std=c++14")
elseif(COMPILER_SUPPORTS_CXX1Y)
  set(CMAKE_CXX_FLAGS "-std=c++1y")
elseif(COMPILER_SUPPORTS_CXX14_WIN)
  set(CMAKE_CXX_FLAGS "/std:c++14")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
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

if(APPLE)
	target_link_libraries(test_domain /usr/local/lib/libgtest.a /usr/local/lib/libghost_static.a Threads::Threads)
	target_link_libraries(test_variable /usr/local/lib/libgtest.a /usr/local/lib/libghost_static.a Threads::Threads)
	target_link_libraries(test_constraint /usr/local/lib/libgtest.a /usr/local/lib/libghost_static.a Threads::Threads)
elseif(WIN32)	
	# target_link_libraries(test_domain gtest.lib ghost.lib Threads::Threads)
	# target_link_libraries(test_variable gtest.lib ghost.lib Threads::Threads)
	# target_link_libraries(test_constraint gtest.lib ghost.lib Threads::Threads)
	target_link_libraries(test_domain gtest ghost Threads::Threads)
	target_link_libraries(test_variable gtest ghost Threads::Threads)
	target_link_libraries(test_constraint gtest ghost Threads::Threads)
else()	
	target_link_libraries(test_domain gtest ghost Threads::Threads)
	target_link_libraries(test_variable gtest ghost Threads::Threads)
	target_link_libraries(test_constraint gtest ghost Threads::Threads)
endif()
	
enable_testing()
add_test( NAME Test_Domain COMMAND test_domain WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
add_test( NAME Test_Variable COMMAND test_variable WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
add_test( NAME Test_Constraint COMMAND test_constraint WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
