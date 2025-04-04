# SDRPlay Wrapper - Streaming API Changes

## Overview

This update significantly improves the streaming API implementation by fixing the callback mechanism between C++ and Python. The primary issue addressed is proper buffer passing from C++ to Python through SWIG, which now allows Python callbacks to receive NumPy arrays directly.

## Changes Made

### 1. SWIG Typemap Integration

- Added `numpy.i` file with standard NumPy typemaps for SWIG
- Created a custom `directorin` typemap for the `handleStreamData` method
- Implemented proper memory management for buffer copying and reference counting

### 2. NumPy Integration

- Updated CMakeLists.txt to properly find and include NumPy
- Added NumPy include directories to the build process
- Implemented proper array creation and copying in the typemap

### 3. Updated Event Callbacks

- Fixed power overload callback to use actual status from the SDRPlay API
- Improved error checking in callback methods

### 4. Test Improvements

- Updated test_sdrplay_streaming.py to verify NumPy array handling
- Uncommented and improved assertions to validate callback functionality
- Added power calculation to verify data integrity

### 5. Example Updates

- Updated example_streaming.py to use the new NumPy array interface
- Improved documentation of the streaming API usage
- Added more robust error handling

### 6. Documentation

- Created CALLBACK_IMPLEMENTATION.md explaining the technical details
- Documented the changes in CHANGES.md (this file)

## Building and Testing

To build with these changes:

```bash
mkdir -p build && cd build
cmake ..
make
```

To test the streaming functionality:

```bash
cd tests
python3 test_sdrplay_streaming.py
```

## Future Work

1. Consider using std::vector in the C++ API for better memory management
2. Add support for different data types beyond 16-bit shorts
3. Improve error handling and recovery in streaming callbacks
4. Implement more comprehensive signal processing examples

## References

- [SDRPlay API Documentation](https://www.sdrplay.com/docs/SDRplay_API_Specification.pdf)
- [SWIG NumPy Documentation](http://www.swig.org/Doc4.0/SWIGDocumentation.html#Python_numpy)
- [NumPy C-API](https://numpy.org/doc/stable/reference/c-api/array.html)