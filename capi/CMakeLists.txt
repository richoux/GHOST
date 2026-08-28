# CMakeLists.txt for GHOST C API

# Minimum required CMake version
cmake_minimum_required(VERSION 3.10)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the C API library target
add_library(ghost_c SHARED
    src/ghost_c_api.cpp
)

# Include directories
target_include_directories(ghost_c
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# Link against the main GHOST library
target_link_libraries(ghost_c PRIVATE ghost)

# Installation rules
install(TARGETS ghost_c
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES include/ghost_c_api.h
    DESTINATION include
)

# Enable testing
enable_testing()

# Add the tests directory
add_subdirectory(tests)
