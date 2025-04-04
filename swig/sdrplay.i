%module sdrplay
%{
#include "device_types.h"
#include "device_parameters.h"
#include "basic_params.h"
#include "control_params.h"
#include "device_params/rsp1a_params.h"
#include "device_params/rspdxr2_params.h"
#include "sdrplay_wrapper.h"
#include "device_registry.h"
#include "device_impl/rsp1a_control.h"
#include "device_impl/rspdxr2_control.h"
#include <memory>
%}

%include <std_string.i>
%include <std_vector.i>
%include <std_map.i>

// Template instantiations for STL containers
%template(DeviceInfoVector) std::vector<sdrplay::DeviceInfo>;

// Enable exceptions
%catches(std::runtime_error);

// Include numpy support for array handling
%include "numpy.i"

%init %{
    import_array();
%}

// Define typemaps for short* arrays in handleStreamData
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

// Define callback interfaces for Python
%feature("director") sdrplay::StreamCallbackHandler;
%feature("director") sdrplay::GainCallbackHandler;
%feature("director") sdrplay::PowerOverloadCallbackHandler;

// Callback handlers - define these in SWIG-space only
%inline %{
namespace sdrplay {
    class StreamCallbackHandler {
    public:
        virtual ~StreamCallbackHandler() {}
        virtual void handleStreamData(short* xi, short* xq, unsigned int numSamples) = 0;
    };
    
    class GainCallbackHandler {
    public:
        virtual ~GainCallbackHandler() {}
        virtual void handleGainChange(int gRdB, int lnaGRdB, float currGain) = 0;
    };
    
    class PowerOverloadCallbackHandler {
    public:
        virtual ~PowerOverloadCallbackHandler() {}
        virtual void handlePowerOverload(bool isOverloaded) = 0;
    };
    
    // Explicitly register device factories for Python
    void initializeDeviceRegistry() {
        DeviceRegistry::registerFactory(RSP1A_HWVER,
            []() { return std::make_unique<RSP1AControl>(); });
        DeviceRegistry::registerFactory(RSPDXR2_HWVER,
            []() { return std::make_unique<RSPdxR2Control>(); });
    }
}
%}

// Ignore implementation details that SWIG can't handle
%ignore sdrplay::Device::Device(Device&&);
%ignore sdrplay::Device::operator=(Device&&);
%ignore sdrplay::Device::pimpl;

// Include headers
%include "device_types.h"
%include "basic_params.h"
%include "control_params.h"
%include "device_params/rsp1a_params.h"
%include "device_params/rspdxr2_params.h"

// Finally include the main wrapper
%include "sdrplay_wrapper.h"
