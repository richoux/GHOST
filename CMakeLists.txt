Cmake_minimum_required (VERSION 3.0)
project (ghost)

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

# compilation options
if(WIN32)
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od")
		add_definitions(-DDEBUG)
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "/O2")
	endif()
else()
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address,undefined,leak -fno-omit-frame-pointer")
		add_definitions(-DDEBUG)
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	endif()
endif()	

# add 'd' at the end of the library name compiled in Debug mode
set(CMAKE_DEBUG_POSTFIX d)

if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wno-unused-parameter")
endif()	

# Options
# Switch between Adaptive Search (default) and experimental search algorithms
option(EXPERIMENTAL "Enable experimental search algorithm" OFF)
if(EXPERIMENTAL)
  add_definitions(-DEXPERIMENTAL)
endif()

OPTION(BENCH "Compile GHOST for benchmarks" OFF)
if(BENCH)
  add_definitions(-DBENCH)
endif()

OPTION(TRACE "Compile GHOST to have a full execution trace, for debugging" OFF)
if(TRACE)
  add_definitions(-DTRACE)
endif()

# add the binary tree to the search path for include files so that we will find headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# add the targets
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# add the library
add_library(ghost SHARED src/domain.cpp src/variable.cpp src/constraint.cpp src/objective.cpp src/solver.cpp)
add_library(ghost_static STATIC src/domain.cpp src/variable.cpp src/constraint.cpp src/objective.cpp src/solver.cpp)

#gather headers list
file(GLOB libHeadersList
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
  )

#gather misc headers list
file(GLOB libMiscHeadersList
  "${CMAKE_CURRENT_SOURCE_DIR}/include/misc/*.hpp"
  )

# add the install targets
install (TARGETS ghost DESTINATION "lib")
install (TARGETS ghost_static DESTINATION "lib")
install (FILES ${libHeadersList} DESTINATION "include/ghost")
install (FILES ${libMiscHeadersList} DESTINATION "include/ghost/misc")

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_NAME "GHOST")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "1")
set (CPACK_PACKAGE_VERSION_MINOR "0")
set (CPACK_PACKAGE_VERSION_PATCH "1")
set (CPACK_PACKAGE_CONTACT "florian.richoux@polytechnique.edu")
set (CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
include (CPack)
