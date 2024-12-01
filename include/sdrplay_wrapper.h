// include/sdrplay_wrapper.h
#ifndef SDRPLAY_WRAPPER_H
#define SDRPLAY_WRAPPER_H

#include "device_control.h"
#include "basic_params.h"
#include "control_params.h"
#include "device_params/rsp1a_params.h"
//#include "device_params/rsp2_params.h"
//#include "device_params/rspduo_params.h"
//#include "device_params/rspdx_params.h"
#include "device_params/rspdxr2_params.h"

namespace sdrplay {

// Forward declarations of callback handlers moved here from old header
class StreamCallbackHandler {
public:
    virtual ~StreamCallbackHandler() = default;
    virtual void handleStreamData(const int16_t* xi, const int16_t* xq, size_t numSamples) = 0;
};

class GainCallbackHandler {
public:
    virtual ~GainCallbackHandler() = default;
    virtual void handleGainChange(int gRdB, int lnaGRdB, double currGain) = 0;
};

class PowerOverloadCallbackHandler {
public:
    virtual ~PowerOverloadCallbackHandler() = default;
    virtual void handlePowerOverload(bool isOverloaded) = 0;
};

// Main Device class becomes a facade for all the components
class Device {
public:
    Device();
    ~Device();

    // Core operations delegated to DeviceControl
    bool open();
    void close();
    float getApiVersion() const;
    std::vector<DeviceInfo> getAvailableDevices();
    bool selectDevice(const DeviceInfo& device);
    bool releaseDevice();

    // Parameter access - returns component instances
    BasicParams* getBasicParams();
    ControlParams* getControlParams();
    Rsp1aParams* getRsp1aParams();

    // Streaming control
    bool startStreamingWithHandlers(
        StreamCallbackHandler* streamHandler = nullptr,
        GainCallbackHandler* gainHandler = nullptr,
        PowerOverloadCallbackHandler* powerHandler = nullptr
    );
    bool stopStreaming();

    // Error handling
    std::string getLastErrorMessage() const;

    // C-style callback entry points
    static void StreamACallback(short* xi, short* xq, sdrplay_api_StreamCbParamsT* params,
                              unsigned int numSamples, unsigned int reset, void* cbContext);
    static void EventCallback(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner,
                            sdrplay_api_EventParamsT* params, void* cbContext);

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace sdrplay

#endif // SDRPLAY_WRAPPER_H
