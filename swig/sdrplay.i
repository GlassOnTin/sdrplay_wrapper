%module sdrplay

// Handle std::vector and std::string first
%include <std_vector.i>
%include <std_string.i>

%{
#include <sdrplay_api.h>
#include "sdrplay_wrapper.h"
using namespace sdrplay;
%}

// Include all the parameter header files
%include "basic_params.h"
%include "control_params.h"
%include "device_params/rsp1a_params.h"

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
%feature("director:except") {
    if ($error != NULL) {
        PyErr_Print();
    }
}

// Single definition of the callback handler implementation and extend
%{
class StreamCallbackHandlerImpl : public sdrplay::StreamCallbackHandler {
public:
    virtual void handleStreamData(const int16_t* xi, const int16_t* xq, size_t numSamples) override {
        // Default implementation
    }
};
%}

%extend sdrplay::StreamCallbackHandler {
    StreamCallbackHandler() { return new StreamCallbackHandlerImpl(); }
}

%feature("director") sdrplay::GainCallbackHandler;
%feature("director") sdrplay::PowerOverloadCallbackHandler;

// Forward declare the DeviceInfo type
%typemap(out) std::vector<sdrplay::DeviceInfo> %{
    $result = SWIG_NewPointerObj(new std::vector<sdrplay::DeviceInfo>($1),
        $descriptor(std::vector<sdrplay::DeviceInfo>*), SWIG_POINTER_OWN);
%}

%template(DeviceInfoVector) std::vector<sdrplay::DeviceInfo>;

// Explicitly declare the DeviceInfo struct for SWIG
namespace sdrplay {
    struct DeviceInfo {
        std::string serialNumber;
        int hwVersion;
        bool isTunerA;
        bool isTunerB;
        bool isRSPDuo;
    };
}

// Add cleanup for vectors
%feature("autodoc", "1");
%feature("newfree") std::vector<sdrplay::DeviceInfo> "delete $this;";
