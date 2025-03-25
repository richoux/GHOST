Cmake_minimum_required (VERSION 3.15)
project(ghost)

set(CMAKE_VERBOSE_MAKEFILE on)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0091 NEW)

# compilation features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF) # prevents cmake compiling with -std=gnu++20 on GNU/Linux instead of a more portable -std=c++20
set(CMAKE_CXX_STANDARD_REQUIRED ON) # prevents a "decay" to C++98 when the compiler does not support C++20

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

# require g++-8, clang 6 or MSVC 19.14
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8")
    message(FATAL_ERROR "GCC version must be at least 8 to compile GHOST.")
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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/error_projection_algorithm.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/error_projection_algorithm_adaptive_search.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/error_projection_algorithm_culprit_search.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/error_projection_algorithm_null.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/space.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/space_of_optimization.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/space_of_violation.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/space_policy.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/space_policy_regular.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/space_policy_switch_optimization.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/value_heuristic.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/value_heuristic_adaptive_search.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/value_heuristic_antidote_search.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/value_heuristic_optimization_space.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/value_heuristic_random_walk.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/variable_candidates_heuristic.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/variable_candidates_heuristic_adaptive_search.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/variable_candidates_heuristic_antidote_search.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/variable_candidates_heuristic_all_free.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/variable_candidates_heuristic_optimization_space.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/variable_heuristic.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/variable_heuristic_antidote_search.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/variable_heuristic_uniform.hpp")

set(libHeadersGlobalConstraintsList
	"${CMAKE_CURRENT_SOURCE_DIR}/include/global_constraints/all_different.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/global_constraints/all_equal.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/global_constraints/fix_value.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/global_constraints/linear_equation.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/global_constraints/linear_equation_eq.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/global_constraints/linear_equation_neq.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/global_constraints/linear_equation_leq.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/global_constraints/linear_equation_geq.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/global_constraints/linear_equation_l.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/global_constraints/linear_equation_g.hpp")

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
	src/algorithms/error_projection_algorithm.cpp
	src/algorithms/error_projection_algorithm_adaptive_search.cpp
	src/algorithms/error_projection_algorithm_culprit_search.cpp
	src/algorithms/error_projection_algorithm_null.cpp
	src/algorithms/space.cpp
	src/algorithms/space_of_optimization.cpp
	src/algorithms/space_of_violation.cpp
	src/algorithms/space_policy.cpp
	src/algorithms/space_policy_regular.cpp
	src/algorithms/space_policy_switch_optimization.cpp
	src/algorithms/value_heuristic_adaptive_search.cpp
	src/algorithms/value_heuristic_antidote_search.cpp
	src/algorithms/value_heuristic_optimization_space.cpp
	src/algorithms/value_heuristic_random_walk.cpp
	src/algorithms/variable_candidates_heuristic_adaptive_search.cpp
	src/algorithms/variable_candidates_heuristic_antidote_search.cpp
	src/algorithms/variable_candidates_heuristic_all_free.cpp
	src/algorithms/variable_candidates_heuristic_optimization_space.cpp
	src/algorithms/variable_heuristic_antidote_search.cpp
	src/algorithms/variable_heuristic_uniform.cpp
	src/global_constraints/all_different.cpp
	src/global_constraints/all_equal.cpp
	src/global_constraints/fix_value.cpp
	src/global_constraints/linear_equation.cpp
	src/global_constraints/linear_equation_eq.cpp
	src/global_constraints/linear_equation_neq.cpp
	src/global_constraints/linear_equation_leq.cpp
	src/global_constraints/linear_equation_geq.cpp
	src/global_constraints/linear_equation_l.cpp
	src/global_constraints/linear_equation_g.cpp)

# add the library
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
	if("${CMAKE_ANDROID_ARCH_ABI}" STREQUAL "arm64-v8a")
		add_library(ghost_android_arm64 SHARED "${toAddInLibs}")
	endif()
	if("${CMAKE_ANDROID_ARCH_ABI}" STREQUAL "armeabi-v7a")
		add_library(ghost_android_armelf SHARED	"${toAddInLibs}")
	endif()
	if("${CMAKE_ANDROID_ARCH_ABI}" STREQUAL "x86")
		add_library(ghost_android_x86 SHARED "${toAddInLibs}")
	endif()
	if("${CMAKE_ANDROID_ARCH_ABI}" STREQUAL "x86_64")
		add_library(ghost_android_x86_64 SHARED	"${toAddInLibs}")
	endif()
else()
	add_library(ghost SHARED "${toAddInLibs}")
	add_library(ghost_static STATIC "${toAddInLibs}")

	target_include_directories(ghost PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/thirdparty/>
  $<INSTALL_INTERFACE:include/ghost/thirdparty>)
endif()

# add the install targets
if(NOT("${CMAKE_SYSTEM_NAME}" STREQUAL "Android"))
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
endif()

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_NAME "GHOST")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "3")
set (CPACK_PACKAGE_VERSION_MINOR "2")
set (CPACK_PACKAGE_VERSION_PATCH "0")
set (CPACK_PACKAGE_CONTACT "florian@richoux.fr")
set (CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
include (CPack)
