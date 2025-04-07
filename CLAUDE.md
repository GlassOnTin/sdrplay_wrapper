# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands
```bash
mkdir -p build && cd build
cmake ..
make
```

## Test Commands
```bash
# C++ tests
cd build
ctest                    # Run all tests
ctest -R test_name       # Run specific test

# Python tests
python -m unittest tests/test_sdrplay_parameters.py  # Run specific test
python -m unittest discover tests                   # Run all tests
```

## Code Style Guidelines
- C++17 standard with modern practices (RAII, smart pointers)
- Namespace: `sdrplay` for all C++ code
- Classes: CamelCase, member functions: camelCase
- Private members with "p" or "impl" prefix
- PIMPL (pointer to implementation) pattern for device implementation
- Header-only declarations with implementation in .cpp files
- Error handling: Return boolean status with error message accessor
- Python bindings generated with SWIG
- Python tests using unittest framework
- Documentation in headers with clear parameter descriptions
- Implement device-specific functionality in device_impl/ subdirectory