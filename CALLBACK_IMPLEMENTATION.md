# SWIG Callback Implementation for SDRPlay Wrapper

This document explains the implementation of C++ to Python callback handling for the SDRPlay wrapper library.

## Overview

The SDRPlay wrapper library uses SWIG to generate Python bindings for the C++ code. One of the key challenges is properly passing data buffers from C++ callbacks to Python code, especially for the streaming API where arrays of IQ samples need to be passed.

## Implementation

### 1. SWIG TypeMaps

We've implemented a custom typemap in `sdrplay.i` to handle the conversion of C++ arrays to Python NumPy arrays:

```cpp
%typemap(directorin) (short* xi, short* xq, unsigned int numSamples) {
    npy_intp dims[1] = { static_cast<npy_intp>($3) };
    
    // Create numpy arrays from the C arrays
    PyObject* xi_array = PyArray_SimpleNewFromData(1, dims, NPY_SHORT, $1);
    PyObject* xq_array = PyArray_SimpleNewFromData(1, dims, NPY_SHORT, $2);
    
    // Make copies to ensure Python doesn't use the memory after it's freed
    PyObject* xi_copy = PyArray_NewCopy((PyArrayObject*)xi_array, NPY_ANYORDER);
    PyObject* xq_copy = PyArray_NewCopy((PyArrayObject*)xq_array, NPY_ANYORDER);
    
    // Set up the Python arguments tuple
    $input = PyTuple_New(3);
    PyTuple_SetItem($input, 0, xi_copy);
    PyTuple_SetItem($input, 1, xq_copy);
    PyTuple_SetItem($input, 2, PyLong_FromLong($3));
    
    // Decrease the reference count for the original temporary arrays
    Py_DECREF(xi_array);
    Py_DECREF(xq_array);
}
```

This typemap is applied to the `handleStreamData` method of the `StreamCallbackHandler` class:

1. It creates NumPy arrays from the C arrays
2. It makes copies of the arrays to ensure Python doesn't use the memory after it's freed by C++
3. It creates a Python tuple with the arrays and the number of samples
4. It cleans up temporary references to avoid memory leaks

### 2. NumPy Integration

We've added NumPy support to the build system:

1. Created a `numpy.i` file with standard NumPy typemaps
2. Updated CMakeLists.txt to find and include NumPy
3. Added NumPy include directories to the build process

### 3. Python Usage

In Python code, the callback methods now receive NumPy arrays:

```python
def handleStreamData(self, xi, xq, numSamples):
    # xi and xq are now NumPy arrays
    power = np.mean(xi**2 + xq**2)
    # Process the data...
```

## Event Callbacks

We've also improved the event callback handling:

1. Updated the power overload callback to use the actual status from the SDRPlay API
2. Added proper error checking in the callback methods

## Testing

Updated test code to:

1. Verify that the arrays received in Python are NumPy arrays
2. Check the shape of the arrays
3. Calculate power to verify the data is valid
4. Uncommented the assertion to check that callbacks are working

## Future Improvements

1. Consider using std::vector in the C++ API instead of raw pointers for better memory management
2. Add support for different data types (not just short)
3. Improve error handling and logging in the callbacks
4. Add more comprehensive tests for the streaming API

## References

- [SWIG NumPy Documentation](http://www.swig.org/Doc4.0/SWIGDocumentation.html#Python_numpy)
- [NumPy C-API](https://numpy.org/doc/stable/reference/c-api/array.html)
- [SWIG Typemaps](http://www.swig.org/Doc4.0/Typemaps.html)