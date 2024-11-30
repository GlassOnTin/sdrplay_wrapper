# CMakeLists.txt
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

# Create wrapper library
add_library(sdrplay_wrapper
    src/sdrplay_wrapper.cpp
)

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

    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

    set_property(SOURCE swig/sdrplay.i PROPERTY CPLUSPLUS ON)
    set_property(SOURCE swig/sdrplay.i PROPERTY SWIG_MODULE_NAME sdrplay)

    swig_add_library(sdrplay_python
        TYPE SHARED
        LANGUAGE python
        SOURCES swig/sdrplay.i
    )

    set_target_properties(sdrplay_python PROPERTIES
        PREFIX ""
        OUTPUT_NAME "_sdrplay"
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    target_include_directories(sdrplay_python PRIVATE
        ${Python3_INCLUDE_DIRS}
    )

    target_link_libraries(sdrplay_python PRIVATE
        sdrplay_wrapper
        ${Python3_LIBRARIES}
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

if(BUILD_PYTHON_BINDINGS)
    install(TARGETS sdrplay_python
        LIBRARY DESTINATION ${Python3_SITEARCH}/sdrplay
    )
    install(FILES ${CMAKE_BINARY_DIR}/sdrplay.py
        DESTINATION ${Python3_SITEARCH}/sdrplay
    )
endif()

# Copy SWIG-generated files to package directory
add_custom_command(TARGET sdrplay_python POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/_sdrplay.so
        ${CMAKE_SOURCE_DIR}/sdrplay/
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/sdrplay.py
        ${CMAKE_SOURCE_DIR}/sdrplay/
)
