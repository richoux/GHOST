Cmake_minimum_required (VERSION 3.0)
project (ghost)

set( CMAKE_VERBOSE_MAKEFILE on )

# require a C++14-capable compiler
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX1Y)
if(COMPILER_SUPPORTS_CXX14)
  set(CMAKE_CXX_FLAGS "-std=c++14")
elseif(COMPILER_SUPPORTS_CXX1Y)
  set(CMAKE_CXX_FLAGS "-std=c++1y")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
endif()

# compilation options
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
  add_definitions(-DDEBUG)
else()
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Switch between Culprit Search (default) and Adaptive Search algorithms
option(ADAPTIVE_SEARCH "Enable Adaptive Search algorithm" OFF)
if(ADAPTIVE_SEARCH)
  add_definitions(-DADAPTIVE_SEARCH)
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
  add_definitions(-DBENCH)
endif()

# add the binary tree to the search path for include files so that we will find headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# add the targets
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# add the library
add_library(ghost SHARED src/domain.cpp src/variable.cpp src/constraint.cpp src/objective.cpp src/solver.cpp)

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
install (FILES ${libHeadersList} DESTINATION "include/ghost")
install (FILES ${libMiscHeadersList} DESTINATION "include/ghost/misc")

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "1")
set (CPACK_PACKAGE_VERSION_MINOR "0")
include (CPack)
