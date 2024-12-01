cmake_minimum_required(VERSION 3.12)
project(sdrplay_wrapper VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set CMake policies for SWIG
if(POLICY CMP0078)
    cmake_policy(SET CMP0078 NEW)
endif()
if(POLICY CMP0086)
    cmake_policy(SET CMP0086 NEW)
endif()

# Find SDRplay API
find_path(SDRPLAY_API_INCLUDE_DIR
    NAMES sdrplay_api.h
    PATHS /usr/local/include
    REQUIRED
)

find_library(SDRPLAY_API_LIBRARY
    NAMES sdrplay_api
    PATHS /usr/local/lib
    REQUIRED
)

# Define source files
set(WRAPPER_SOURCES
    src/sdrplay_wrapper.cpp
    src/device_control.cpp
    src/basic_params.cpp
    src/control_params.cpp
    src/device_params/rsp1a_params.cpp
)

# Create wrapper library
add_library(sdrplay_wrapper ${WRAPPER_SOURCES})

target_include_directories(sdrplay_wrapper
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${SDRPLAY_API_INCLUDE_DIR}
)

target_link_libraries(sdrplay_wrapper
    PRIVATE
        ${SDRPLAY_API_LIBRARY}
)

# Handle SWIG Python bindings
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)
if(BUILD_PYTHON_BINDINGS)
    find_package(SWIG REQUIRED)
    include(${SWIG_USE_FILE})

    find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED)

    # Setup SWIG
    set(CMAKE_SWIG_FLAGS "")
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${SDRPLAY_API_INCLUDE_DIR}
        ${Python3_NumPy_INCLUDE_DIRS}
    )

    set_property(SOURCE swig/sdrplay.i PROPERTY CPLUSPLUS ON)
    set_property(SOURCE swig/sdrplay.i PROPERTY SWIG_MODULE_NAME sdrplay)

    # Create Python module
    swig_add_library(sdrplay_python
        TYPE SHARED
        LANGUAGE python
        SOURCES swig/sdrplay.i
    )

    set_target_properties(sdrplay_python PROPERTIES
        PREFIX ""
        OUTPUT_NAME "_sdrplay"
        # Output directly to the package directory for pip install
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/sdrplay
    )

    target_include_directories(sdrplay_python PRIVATE
        ${Python3_INCLUDE_DIRS}
        ${Python3_NumPy_INCLUDE_DIRS}
    )

    target_link_libraries(sdrplay_python PRIVATE
        sdrplay_wrapper
        ${Python3_LIBRARIES}
    )

    # Clean up old files before build
    add_custom_target(clean_python
        COMMAND ${CMAKE_COMMAND} -E remove_directory
            ${CMAKE_SOURCE_DIR}/sdrplay
        COMMAND ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_SOURCE_DIR}/sdrplay
    )
    add_dependencies(sdrplay_python clean_python)

    # Copy generated files to package directory
    add_custom_command(TARGET sdrplay_python POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_BINARY_DIR}/sdrplay.py
            ${CMAKE_SOURCE_DIR}/sdrplay/sdrplay.py
        COMMAND ${CMAKE_COMMAND} -E echo "from .sdrplay import *" >
            ${CMAKE_SOURCE_DIR}/sdrplay/__init__.py
    )
endif()

# Testing configuration
enable_testing()

# C++ tests
add_executable(test_sdrplay_api tests/test_sdrplay_api.cpp)
target_link_libraries(test_sdrplay_api PRIVATE sdrplay_wrapper)
add_test(NAME test_sdrplay_api COMMAND test_sdrplay_api)

# Python tests
if(BUILD_PYTHON_BINDINGS)
    add_test(NAME test_sdrplay_python
        COMMAND ${Python3_EXECUTABLE} -m pytest ${CMAKE_SOURCE_DIR}/tests/test_sdrplay.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    add_test(NAME test_sdrplay_parameters
        COMMAND ${Python3_EXECUTABLE} -m pytest ${CMAKE_SOURCE_DIR}/tests/test_sdrplay_parameters.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    # Set Python path for tests
    set_tests_properties(
        test_sdrplay_python
        test_sdrplay_parameters
        PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_SOURCE_DIR}"
    )
endif()

# Installation rules
include(GNUInstallDirs)

install(TARGETS sdrplay_wrapper
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sdrplay
    FILES_MATCHING PATTERN "*.h"
)

# Clean-up
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/sdrplay/__pycache__
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/sdrplay/_sdrplay.so
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/sdrplay/sdrplay.py
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/tests/__pycache__
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/tests/test_sdrplay_api
)
