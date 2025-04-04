// include/sdrplay_wrapper.h
#pragma once
#include <memory>
#include <vector>
#include "device_types.h"

namespace sdrplay {

// Forward declarations
class StreamCallbackHandler;
class GainCallbackHandler;
class PowerOverloadCallbackHandler;
class RSP1AParameters;
class RSPdxR2Parameters;
using Rsp1aParams = RSP1AParameters; 
using RspDxR2Params = RSPdxR2Parameters;

class Device {
public:
    Device();
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    Device(Device&&) = default;
    Device& operator=(Device&&) = default;

    bool selectDevice(const DeviceInfo& deviceInfo);
    bool releaseDevice();
    std::vector<DeviceInfo> getAvailableDevices();

    void setFrequency(double freq);
    double getFrequency() const;
    void setSampleRate(double rate);
    double getSampleRate() const;
    
    // Device parameters accessors
    Rsp1aParams* getRsp1aParams();
    RspDxR2Params* getRspDxR2Params();

    // Streaming support
    bool registerStreamCallback(StreamCallbackHandler* handler);
    bool registerGainCallback(GainCallbackHandler* handler);
    bool registerPowerOverloadCallback(PowerOverloadCallbackHandler* handler);
    bool startStreaming();
    bool stopStreaming();
    bool isStreaming() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace sdrplay
