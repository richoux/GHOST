Cmake_minimum_required (VERSION 3.15)
project(ghost)

set(CMAKE_VERBOSE_MAKEFILE on)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0091 NEW)

# compilation features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF) # prevents cmake compiling with -std=gnu++17 on GNU/Linux instead of a more portable -std=c++17
set(CMAKE_CXX_STANDARD_REQUIRED ON) # prevents a "decay" to C++98 when the compiler does not support C++17

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

# require g++-7, clang 6 or MSVC 19.14
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
option(NO_ASAN "Disable fsanitizer" OFF)
if(NO_ASAN)
	set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
endif()

# add the binary tree to the search path for include files so that we will find headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# gather headers lists
set(libHeadersList
	"${CMAKE_CURRENT_SOURCE_DIR}/include/variable.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/constraint.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/objective.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/auxiliary_data.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/model.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/model_builder.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/search_unit.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/search_unit_data.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/solver.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/options.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/print.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/macros.hpp")

set(libHeadersAlgorithmsList
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/variable_heuristic.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/variable_candidates_heuristic.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/value_heuristic.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/error_projection_heuristic.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/adaptive_search_variable_heuristic.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/adaptive_search_variable_candidates_heuristic.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/adaptive_search_value_heuristic.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/adaptive_search_error_projection_heuristic.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/antidote_search_variable_heuristic.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/antidote_search_variable_candidates_heuristic.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/antidote_search_value_heuristic.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/culprit_search_error_projection_heuristic.hpp")

set(libHeadersGlobalConstraintsList
	"${CMAKE_CURRENT_SOURCE_DIR}/include/global_constraints/all_different.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/global_constraints/linear_equation.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/global_constraints/fix_value.hpp")

set(libExternalHeadersList
	"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/randutils.hpp")

# add the targets
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(toAddInLibs
	src/variable.cpp
	src/constraint.cpp
	src/objective.cpp
	src/auxiliary_data.cpp
	src/model.cpp
	src/model_builder.cpp
	src/options.cpp
	src/print.cpp
	src/algorithms/adaptive_search_variable_heuristic.cpp
	src/algorithms/adaptive_search_variable_candidates_heuristic.cpp
	src/algorithms/adaptive_search_value_heuristic.cpp
	src/algorithms/adaptive_search_error_projection_heuristic.cpp
	src/algorithms/antidote_search_variable_heuristic.cpp
	src/algorithms/antidote_search_variable_candidates_heuristic.cpp
	src/algorithms/antidote_search_value_heuristic.cpp
	src/algorithms/culprit_search_error_projection_heuristic.cpp
	src/global_constraints/all_different.cpp
	src/global_constraints/fix_value.cpp
	src/global_constraints/linear_equation.cpp)	

# add the library
add_library(ghost SHARED
	"${toAddInLibs}")

add_library(ghost_static STATIC
	"${toAddInLibs}")

target_include_directories(ghost PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/thirdparty/>
  $<INSTALL_INTERFACE:include/ghost/thirdparty>)

# add the install targets
if(CMAKE_SIZEOF_VOID_P EQUAL 4) # 32bits
	install (TARGETS ghost DESTINATION "lib32")
	install (TARGETS ghost_static DESTINATION "lib32")
else() # 64bits
	install (TARGETS ghost DESTINATION "lib")
	install (TARGETS ghost_static DESTINATION "lib")
endif()
install (FILES ${libHeadersList} DESTINATION "include/ghost")
install (FILES ${libHeadersAlgorithmsList} DESTINATION "include/ghost/algorithms")
install (FILES ${libHeadersGlobalConstraintsList} DESTINATION "include/ghost/global_constraints")
install (FILES ${libExternalHeadersList} DESTINATION "include/ghost/thirdparty")

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_NAME "GHOST")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "2")
set (CPACK_PACKAGE_VERSION_MINOR "4")
set (CPACK_PACKAGE_VERSION_PATCH "0")
set (CPACK_PACKAGE_CONTACT "florian.richoux@polytechnique.edu")
set (CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
include (CPack)
