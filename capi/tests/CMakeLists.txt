# CMakeLists.txt for GHOST C API tests

# Add the test executable
add_executable(test_ghost_c_api test_ghost_c_api.c)

# Include directories
target_include_directories(test_ghost_c_api
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Link against the GHOST C API library
target_link_libraries(test_ghost_c_api PRIVATE ghost_c)

# Add a test that runs the executable
add_test(NAME test_ghost_c_api COMMAND test_ghost_c_api)

# Set the output directory for the test executable
set_target_properties(test_ghost_c_api
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin"
)
