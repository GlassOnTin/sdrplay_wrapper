#pragma once
#include "device_control.h"
#include <string>
#include <memory>

namespace sdrplay {

// Base class for device parameters
class DeviceParameters {
public:
    virtual ~DeviceParameters() = default;
    
    virtual std::string getDeviceName() const = 0;
    virtual void applyDefaults() = 0;
    
    // Common parameter methods
    virtual void setFrequency(double freq) = 0;
    virtual double getFrequency() const = 0;
    virtual void setSampleRate(double rate) = 0;
    virtual double getSampleRate() const = 0;
};

} // namespace sdrplay