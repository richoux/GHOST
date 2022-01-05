Cmake_minimum_required (VERSION 3.15)
project (ghost)

set( CMAKE_VERBOSE_MAKEFILE on )
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0091 NEW)

# require a C++17-capable compiler
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
CHECK_CXX_COMPILER_FLAG("-std=c++1z" COMPILER_SUPPORTS_CXX1Z)
CHECK_CXX_COMPILER_FLAG("/std:c++17" COMPILER_SUPPORTS_CXX17_WIN)
if(COMPILER_SUPPORTS_CXX17)
  set(CMAKE_CXX_FLAGS "-std=c++17")
elseif(COMPILER_SUPPORTS_CXX1Z)
  set(CMAKE_CXX_FLAGS "-std=c++1z")
elseif(COMPILER_SUPPORTS_CXX17_WIN)
  set(CMAKE_CXX_FLAGS "/std:c++17")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support. Please use a different C++ compiler.")
endif()

# require g++-7, clang 6 or or MSVC 19.14
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7")
    message(FATAL_ERROR "GCC version must be at least 7 to compile GHOST.")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6")
    message(FATAL_ERROR "Clang version must be at least 6 to compile GHOST.")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.14")
    message(FATAL_ERROR "MSVC version must be at least 19.14 to compile GHOST.")
  endif()
endif()

# compilation options
if(WIN32)
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od")
		add_definitions(-DGHOST_DEBUG)
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "/O2")
	endif()
	set(CMAKE_INSTALL_PREFIX "C:/Users/Public/Documents/ghost")
else()
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-omit-frame-pointer -fsanitize=address,undefined,leak -static-libasan")
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

# cmake --install on visual studio
if("${WIN32}")
  set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD 1)
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
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# add the targets
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# add the library
add_library(ghost SHARED src/variable.cpp src/constraint.cpp src/objective.cpp src/auxiliary_data.cpp src/model.cpp src/model_builder.cpp src/options.cpp src/print.cpp)
add_library(ghost_static STATIC src/variable.cpp src/constraint.cpp src/objective.cpp src/auxiliary_data.cpp src/model.cpp src/model_builder.cpp src/options.cpp src/print.cpp)

#gather headers list
file(GLOB libHeadersList
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
  )

#gather external headers list
file(GLOB libExternalHeadersList
  "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/randutils.hpp"
  )

target_include_directories(ghost PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/thirdparty/>
  $<INSTALL_INTERFACE:include/ghost/thirdparty>
)

# add the install targets
if(CMAKE_SIZEOF_VOID_P EQUAL 4) # 32bits
	install (TARGETS ghost DESTINATION "lib32")
	install (TARGETS ghost_static DESTINATION "lib32")
else() # 64bits
	install (TARGETS ghost DESTINATION "lib")
	install (TARGETS ghost_static DESTINATION "lib")
endif()
install (FILES ${libHeadersList} DESTINATION "include/ghost")
install (FILES ${libExternalHeadersList} DESTINATION "include/ghost/thirdparty")

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_NAME "GHOST")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "2")
set (CPACK_PACKAGE_VERSION_MINOR "2")
set (CPACK_PACKAGE_VERSION_PATCH "1")
set (CPACK_PACKAGE_CONTACT "florian.richoux@polytechnique.edu")
set (CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
include (CPack)
