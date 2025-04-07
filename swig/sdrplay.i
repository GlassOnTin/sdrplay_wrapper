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
#include "callback_wrapper.h"
#include "device_impl/rsp1a_control.h"
#include "device_impl/rspdxr2_control.h"
#include <memory>
#include <complex>
%}

%include <std_string.i>
%include <std_vector.i>
%include <std_map.i>
%include <std_complex.i>
%include "numpy.i"

// Template instantiations for STL containers
%template(DeviceInfoVector) std::vector<sdrplay::DeviceInfo>;
%template(ComplexShortVector) std::vector<std::complex<short>>;

// Enable exceptions
%catches(std::runtime_error);

%init %{
    import_array();
%}

%inline %{
namespace sdrplay {
    // Explicitly register device factories for Python
    void initializeDeviceRegistry() {
        DeviceRegistry::registerFactory(RSP1A_HWVER,
            []() { return std::make_unique<RSP1AControl>(); });
        DeviceRegistry::registerFactory(RSPDXR2_HWVER,
            []() { return std::make_unique<RSPdxR2Control>(); });
    }
    
    // Helper function to create a NumPy array from buffer
    PyObject* samples_to_numpy(const std::complex<short>* samples, size_t count) {
        npy_intp dims[1] = { static_cast<npy_intp>(count) };
        PyObject* array = PyArray_SimpleNewFromData(1, dims, NPY_COMPLEX64, 
                                             (void*)samples);
        // Make the data copy so we can return it safely
        PyObject* result = PyArray_Copy((PyArrayObject*)array);
        Py_DECREF(array);
        return result;
    }
    
    // Simple buffer class for Python to allocate and hold sample data
    class PythonSampleBuffer {
    public:
        PythonSampleBuffer(size_t size) : data(size) {}
        
        const std::complex<short>* getData() const {
            return data.data();
        }
        
        size_t getSize() const {
            return data.size();
        }
        
        PyObject* toNumpy() const {
            return samples_to_numpy(data.data(), data.size());
        }
        
    private:
        std::vector<std::complex<short>> data;
    };
}
%}

// Define callback types
%feature("director") SampleCallbackHandler;
%feature("director") EventCallbackHandler;

%inline %{
namespace sdrplay {
    class SampleCallbackHandler {
    public:
        virtual ~SampleCallbackHandler() {}
        virtual void handleSamples(const std::complex<short>* samples, size_t count) = 0;
    };
    
    class EventCallbackHandler {
    public:
        virtual ~EventCallbackHandler() {}
        virtual void handleEvent(sdrplay::EventType type, const sdrplay::EventParams& params) = 0;
    };
}
%}

// Ignore implementation details that SWIG can't handle
%ignore sdrplay::Device::Device(Device&&);
%ignore sdrplay::Device::operator=(Device&&);
%ignore sdrplay::Device::pimpl;
%ignore sdrplay::CallbackWrapper::streamCallback;
%ignore sdrplay::CallbackWrapper::eventCallback;
%ignore sdrplay::CallbackWrapper::processStreamCallback;
%ignore sdrplay::CallbackWrapper::processEventCallback;
%ignore sdrplay::CallbackWrapper::getStreamCallback;
%ignore sdrplay::CallbackWrapper::getEventCallback;
%ignore sdrplay::CallbackWrapper::getContext;

// Include headers
%include "device_types.h"
%include "callback_wrapper.h"
%include "basic_params.h"
%include "control_params.h"
%include "device_params/rsp1a_params.h"
%include "device_params/rspdxr2_params.h"

// Create typemaps for complex array handling
%numpy_typemaps(std::complex<short>, NPY_COMPLEX64)

// Add methods to Device for setting callbacks from Python
%extend sdrplay::Device {
    void setPythonSampleCallback(SampleCallbackHandler* handler) {
        if (handler) {
            $self->setSampleCallback([handler](const std::complex<short>* samples, size_t count) {
                handler->handleSamples(samples, count);
            });
        } else {
            $self->setSampleCallback(nullptr);
        }
    }
    
    void setPythonEventCallback(EventCallbackHandler* handler) {
        if (handler) {
            $self->setEventCallback([handler](sdrplay::EventType type, const sdrplay::EventParams& params) {
                handler->handleEvent(type, params);
            });
        } else {
            $self->setEventCallback(nullptr);
        }
    }
    
    // Convenience method for Python to read samples directly to a NumPy array
    PyObject* readSamplesToNumpy(size_t maxCount) {
        // Create a buffer to hold the samples
        std::vector<std::complex<short>> buffer(maxCount);
        
        // Read the samples
        size_t count = $self->readSamples(buffer.data(), maxCount);
        
        // If no samples were read, return an empty NumPy array
        if (count == 0) {
            npy_intp dims[1] = { 0 };
            return PyArray_SimpleNew(1, dims, NPY_COMPLEX64);
        }
        
        // Create a NumPy array with the samples
        return sdrplay::samples_to_numpy(buffer.data(), count);
    }
}

// Finally include the main wrapper
%include "sdrplay_wrapper.h"