%module sdrplay

%{
#include "sdrplay_wrapper.h"
%}

// Handle std::vector
%include <std_vector.i>
%include <std_string.i>

// Handle callbacks using directors
%feature("director") StreamCallback;
%feature("director") GainCallback;
%feature("director") PowerOverloadCallback;

// Handle std::function callbacks
%ignore StreamCallback;
%ignore GainCallback;
%ignore PowerOverloadCallback;

// Template instantiations for vectors
%template(DeviceInfoVector) std::vector<sdrplay::Device::DeviceInfo>;

// Main wrapper class
%include "sdrplay_wrapper.h"
