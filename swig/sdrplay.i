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
%feature("director") StreamCallbackHandler {
    virtual ~StreamCallbackHandler() {}
    virtual void handleStreamData(const int16_t* xi, const int16_t* xq, size_t numSamples) = 0;
};

%feature("director") GainCallbackHandler {
    virtual ~GainCallbackHandler() {}
    virtual void handleGainChange(int gRdB, int lnaGRdB, double currGain) = 0;
};

%feature("director") PowerOverloadCallbackHandler {
    virtual ~PowerOverloadCallbackHandler() {}
    virtual void handlePowerOverload(bool isOverloaded) = 0;
};

// Define the callback handler classes
%inline %{
class StreamCallbackHandler {
public:
    virtual ~StreamCallbackHandler() {}
    virtual void handleStreamData(const int16_t* xi, const int16_t* xq, size_t numSamples) = 0;
};

class GainCallbackHandler {
public:
    virtual ~GainCallbackHandler() {}
    virtual void handleGainChange(int gRdB, int lnaGRdB, double currGain) = 0;
};

class PowerOverloadCallbackHandler {
public:
    virtual ~PowerOverloadCallbackHandler() {}
    virtual void handlePowerOverload(bool isOverloaded) = 0;
};
%}

// Extend Device class with callback handler setters
%extend sdrplay::Device {
    bool startStreamingWithHandlers(StreamCallbackHandler* streamHandler = nullptr,
                                  GainCallbackHandler* gainHandler = nullptr,
                                  PowerOverloadCallbackHandler* powerHandler = nullptr) {
        return $self->startStreaming(
            streamHandler ? [streamHandler](const int16_t* xi, const int16_t* xq, size_t numSamples) {
                streamHandler->handleStreamData(xi, xq, numSamples);
            } : StreamCallback(),
            gainHandler ? [gainHandler](int gRdB, int lnaGRdB, double currGain) {
                gainHandler->handleGainChange(gRdB, lnaGRdB, currGain);
            } : GainCallback(),
            powerHandler ? [powerHandler](bool isOverloaded) {
                powerHandler->handlePowerOverload(isOverloaded);
            } : PowerOverloadCallback()
        );
    }
}

// Template instantiations for vectors
namespace std {
    %template(DeviceInfoVector) vector<sdrplay::DeviceInfo>;
}

// Include the wrapper header
%include "sdrplay_wrapper.h"
