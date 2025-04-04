#!/bin/bash
set -e

# Get the script directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$DIR/.."

echo "Compiling device registry test..."

# Check where the SDRPlay API is installed
if [ -d "/usr/local/include" ] && [ -f "/usr/local/include/sdrplay_api.h" ]; then
    API_INCLUDE="-I/usr/local/include"
    echo "Using SDRPlay API from /usr/local/include"
else
    API_INCLUDE=""
    echo "Warning: SDRPlay API not found in /usr/local/include"
fi

# Compile test using g++
g++ -std=c++17 -g -o tests/test_device_registry_standalone \
    tests/test_device_registry_standalone.cpp \
    src/device_registry.cpp \
    src/device_control.cpp \
    -I include \
    $API_INCLUDE \
    -DSDRPLAY_TESTING

# Make the test executable
chmod +x tests/test_device_registry_standalone

echo "Running device registry test..."
tests/test_device_registry_standalone

echo "Test complete!"