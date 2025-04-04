cmake_minimum_required(VERSION 3.12)
project(sdrplay_wrapper VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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
    src/device.cpp
    src/device_control.cpp
    src/device_registry.cpp
    src/basic_params.cpp
    src/control_params.cpp
    src/device_impl/rsp1a_control.cpp
    src/device_impl/rspdxr2_control.cpp
)

# Create library target
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

# Testing configuration
enable_testing()

add_executable(test_sdrplay_api tests/test_sdrplay_api.cpp)
target_link_libraries(test_sdrplay_api PRIVATE sdrplay_wrapper)
add_test(NAME test_sdrplay_api COMMAND test_sdrplay_api)

add_executable(test_device_registry tests/test_device_registry.cpp)
target_link_libraries(test_device_registry PRIVATE sdrplay_wrapper)
add_test(NAME test_device_registry COMMAND test_device_registry)

# Python bindings (SWIG)
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)

if(BUILD_PYTHON_BINDINGS)
    # Set CMake policies for modern SWIG
    if(POLICY CMP0078)
        cmake_policy(SET CMP0078 NEW)
    endif()
    
    if(POLICY CMP0086)
        cmake_policy(SET CMP0086 NEW)
    endif()

    # Find Python, NumPy, and SWIG
    find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED)
    find_package(SWIG REQUIRED)
    include(${SWIG_USE_FILE})
    
    # Get NumPy include directory
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import numpy; print(numpy.get_include())"
        OUTPUT_VARIABLE NUMPY_INCLUDE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Configure SWIG
    set(CMAKE_SWIG_FLAGS "")
    
    # Create Python module
    set(PYTHON_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/sdrplay")
    
    # Set up SWIG interface module
    include_directories(${Python3_INCLUDE_DIRS})
    include_directories(${NUMPY_INCLUDE_DIR})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/swig)
    
    set_source_files_properties(swig/sdrplay.i PROPERTIES
        CPLUSPLUS ON
        SWIG_MODULE_NAME sdrplay
    )
    
    swig_add_library(sdrplay_python
        TYPE MODULE
        LANGUAGE python
        SOURCES swig/sdrplay.i
    )
    
    swig_link_libraries(sdrplay_python
        sdrplay_wrapper
        ${Python3_LIBRARIES}
        ${SDRPLAY_API_LIBRARY}
    )
    
    # Install SWIG-generated files to the correct location
    add_custom_command(TARGET sdrplay_python POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy 
            "${CMAKE_CURRENT_BINARY_DIR}/sdrplay.py" 
            "${PYTHON_MODULE_PATH}/sdrplay.py"
        COMMAND ${CMAKE_COMMAND} -E echo "from .sdrplay import *" > "${PYTHON_MODULE_PATH}/__init__.py"
    )
    
    # Clean Python files
    add_custom_target(clean_python
        COMMAND ${CMAKE_COMMAND} -E remove -f
            "${PYTHON_MODULE_PATH}/sdrplay.py"
            "${PYTHON_MODULE_PATH}/_sdrplay.so"
            "${PYTHON_MODULE_PATH}/__init__.py"
    )
    
    # Set module output directory
    set_target_properties(sdrplay_python PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PYTHON_MODULE_PATH}"
        OUTPUT_NAME _sdrplay
        PREFIX ""
        SUFFIX .so
    )
endif()
