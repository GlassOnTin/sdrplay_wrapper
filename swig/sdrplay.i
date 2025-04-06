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

%inline %{
namespace sdrplay {
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
