%module sdrplay

%{
#include "sdrplay_wrapper.h"
using namespace sdrplay;
%}

// Handle std::vector and std::string
%include <std_vector.i>
%include <std_string.i>

// Handle exceptions
%include "exception.i"
%exception {
    try {
        $action
    }
    catch (const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

// Create Python callback interfaces
%feature("director") sdrplay::StreamCallbackHandler;
%feature("director") sdrplay::GainCallbackHandler;
%feature("director") sdrplay::PowerOverloadCallbackHandler;

// Template instantiations for vectors
namespace std {
    %template(DeviceInfoVector) vector<sdrplay::DeviceInfo>;
}

// Include the wrapper header
%include "sdrplay_wrapper.h"
