Cmake_minimum_required (VERSION 3.0)
project (ghost)

set( CMAKE_VERBOSE_MAKEFILE on )

# require a C++20-capable compiler
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)
CHECK_CXX_COMPILER_FLAG("-std=c++2a" COMPILER_SUPPORTS_CXX2A)
CHECK_CXX_COMPILER_FLAG("/std:c++20" COMPILER_SUPPORTS_CXX20_WIN)
if(COMPILER_SUPPORTS_CXX20)
  set(CMAKE_CXX_FLAGS "-std=c++20")
elseif(COMPILER_SUPPORTS_CXX2A)
  set(CMAKE_CXX_FLAGS "-std=c++2a")
elseif(COMPILER_SUPPORTS_CXX20_WIN)
  set(CMAKE_CXX_FLAGS "/std:c++20")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++20 support. Please use a different C++ compiler.")
endif()

# require g++-10, or MSVC 19.28 (for jthread). Clang does not support jthread yet.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10")
    message(FATAL_ERROR "GCC version must be at least 10 to compile GHOST.")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.28")
    message(FATAL_ERROR "MSVC version must be at least 19.28 to compile GHOST.")
  endif()
#elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
endif()

# compilation options
if(WIN32)
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od")
		add_definitions(-DGHOST_DEBUG)
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "/O2")
	endif()
else()
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address,undefined,leak -fno-omit-frame-pointer -static-libasan")
		add_definitions(-DGHOST_DEBUG)
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
option(GHOST_EXPERIMENTAL "Enable experimental search algorithm" OFF)
if(GHOST_EXPERIMENTAL)
  add_definitions(-DGHOST_EXPERIMENTAL)
endif()

option(NO_ASAN "Disable fsanitizer" OFF)
if(NO_ASAN)
	set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
endif()


# add the binary tree to the search path for include files so that we will find headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# add the targets
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# add the library
add_library(ghost SHARED src/variable.cpp src/constraint.cpp src/objective.cpp src/misc/print.cpp)
add_library(ghost_static STATIC src/variable.cpp src/constraint.cpp src/objective.cpp src/misc/print.cpp)

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
set (CPACK_PACKAGE_VERSION_MAJOR "2")
set (CPACK_PACKAGE_VERSION_MINOR "0")
set (CPACK_PACKAGE_VERSION_PATCH "0")
set (CPACK_PACKAGE_CONTACT "florian.richoux@polytechnique.edu")
set (CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
include (CPack)
