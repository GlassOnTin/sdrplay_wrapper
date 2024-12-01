# tests/CMakeLists.txt
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# Copy test files to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/test_sdrplay.py
    ${CMAKE_BINARY_DIR}/tests/test_sdrplay.py
    COPYONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/test_sdrplay_parameters.py
    ${CMAKE_BINARY_DIR}/tests/test_sdrplay_parameters.py
    COPYONLY
)

# Add test command - using explicit test files like the Makefile
add_test(
    NAME python_tests
    COMMAND ${Python3_EXECUTABLE} -m unittest test_sdrplay.py test_sdrplay_parameters.py -v
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# Set environment variables for test - include both build and source paths
set_tests_properties(python_tests PROPERTIES
    ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}:${CMAKE_SOURCE_DIR}"
)
