cmake_minimum_required (VERSION 3.1)
project (ghost_tutorial_video)

set( CMAKE_VERBOSE_MAKEFILE on )

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

if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
	INCLUDE_DIRECTORIES("C:/Program Files (x86)/ghost/include")
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

add_executable( tutorial_knapsack src/knapsack_alldiff.cpp src/knapsack_capacity.cpp src/knapsack_model_builder.cpp src/knapsack_objective.cpp src/knapsack.cpp )
target_compile_definitions( tutorial_knapsack PUBLIC -DGHOST_BENCH )

if(APPLE)
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		target_link_libraries(tutorial_knapsack /usr/local/lib/libghost_staticd.a Threads::Threads)
	else()
		target_link_libraries(tutorial_knapsack /usr/local/lib/libghost_static.a Threads::Threads)
	endif()
else()	
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		target_link_libraries(tutorial_knapsack ghost Threads::Threads)
	else()
		target_link_libraries(tutorial_knapsack ghost_static Threads::Threads)
	endif()
endif()
