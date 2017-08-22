cmake_minimum_required (VERSION 2.6)
project (ghost)

# set(CMAKE_BUILD_TYPE Release)

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

# right compilation option while debug mode
if(CMAKE_BUILD_TYPE EQUAL "DEBUG")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

# compilation options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -Wall")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")  
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()
  
# add the binary tree to the search path for include files
# so that we will find headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# add the targets
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)


# add the library
add_library(ghost SHARED src/variable.cpp src/domain.cpp src/constraint.cpp src/objective.cpp src/solver.cpp)

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
