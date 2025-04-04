#!/bin/bash
set -e

# Get the script directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$DIR/.."

echo "Compiling device registry mock test..."

# Compile test using g++
g++ -std=c++17 -g -o tests/test_mock_device_registry \
    tests/test_mock_device_registry.cpp \
    -DSDRPLAY_TESTING

# Make the test executable
chmod +x tests/test_mock_device_registry

echo "Running mock device registry test..."
tests/test_mock_device_registry

echo "Test complete!"