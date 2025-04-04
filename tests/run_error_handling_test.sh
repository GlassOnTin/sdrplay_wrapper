#!/bin/bash
# Compile and run the error handling test

set -e
cd "$(dirname "$0")/.."

# Make sure the build directory exists
mkdir -p build
cd build

# Build the test
cmake ..
make test_error_handling

# Run the test
./test_error_handling