#include "device_impl/rspdxr2_control.h"
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <map>

// Instead of including sdrplay_wrapper.h, define the necessary interface classes inline
namespace sdrplay {

// Forward declare the callback handler classes with their methods used in this file
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

// Global mapping to track device instances for callbacks
std::mutex rspdxr2DeviceMapMutex;
std::map<void*, RSPdxR2Control*> rspdxr2DeviceMap;

struct RSPdxR2Control::RSPdxR2Impl {
    sdrplay_api_DeviceParamsT* deviceParams{nullptr};
    sdrplay_api_RxChannelParamsT* channelParams{nullptr};
    double currentFreq{100.0e6};
    double currentSampleRate{2.0e6};
    bool hdrMode{false};
    bool biasTEnabled{false};
    bool isStreaming{false};
    
    // Callback handlers
    StreamCallbackHandler* streamCallback{nullptr};
    GainCallbackHandler* gainCallback{nullptr};
    PowerOverloadCallbackHandler* powerCallback{nullptr};
    
    // SDRPlay API callback functions
    sdrplay_api_CallbackFnsT callbackFns;
};

RSPdxR2Control::RSPdxR2Control() : impl(std::make_unique<RSPdxR2Impl>()) {}
RSPdxR2Control::~RSPdxR2Control() = default;

void RSPdxR2Control::setFrequency(double freq) {
    if (impl->channelParams) {
        impl->channelParams->tunerParams.rfFreq.rfHz = freq;
        impl->currentFreq = freq;

        // Update the device parameters
        auto* device = getCurrentDevice();
        if (device) {
            sdrplay_api_Update(device->dev, device->tuner,
                             sdrplay_api_Update_Tuner_Frf,
                             sdrplay_api_Update_Ext1_None);
        }
    }
}

double RSPdxR2Control::getFrequency() const {
    return impl->currentFreq;
}

void RSPdxR2Control::setSampleRate(double rate) {
    if (impl->deviceParams) {
        impl->deviceParams->devParams->fsFreq.fsHz = rate;
        impl->currentSampleRate = rate;

        // Update the device parameters
        auto* device = getCurrentDevice();
        if (device) {
            sdrplay_api_Update(device->dev, device->tuner,
                             sdrplay_api_Update_Dev_Fs,
                             sdrplay_api_Update_Ext1_None);
        }
    }
}

double RSPdxR2Control::getSampleRate() const {
    return impl->currentSampleRate;
}

void RSPdxR2Control::setHDRMode(bool enable) {
    if (impl->deviceParams) {
        impl->deviceParams->devParams->rspDxParams.hdrEnable = enable;
        impl->hdrMode = enable;

        // Update the device parameters
        auto* device = getCurrentDevice();
        if (device) {
            sdrplay_api_Update(device->dev, device->tuner,
                             sdrplay_api_Update_None,
                             sdrplay_api_Update_RspDx_HdrEnable);
        }
    }
}

void RSPdxR2Control::setBiasTEnabled(bool enable) {
    if (impl->deviceParams) {
        impl->deviceParams->devParams->rspDxParams.biasTEnable = enable;
        impl->biasTEnabled = enable;

        // Update the device parameters
        auto* device = getCurrentDevice();
        if (device) {
            sdrplay_api_Update(device->dev, device->tuner,
                             sdrplay_api_Update_None,
                             sdrplay_api_Update_RspDx_BiasTControl);
        }
    }
}

// Static callback implementations
void RSPdxR2Control::streamCallback(short* xi, short* xq, sdrplay_api_StreamCbParamsT* params, 
                                  unsigned int numSamples, unsigned int reset, void* cbContext) {
    std::lock_guard<std::mutex> lock(rspdxr2DeviceMapMutex);
    auto it = rspdxr2DeviceMap.find(cbContext);
    if (it != rspdxr2DeviceMap.end()) {
        RSPdxR2Control* instance = it->second;
        if (instance && instance->impl->streamCallback) {
            instance->impl->streamCallback->handleStreamData(xi, xq, numSamples);
        }
    }
}

void RSPdxR2Control::eventCallback(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner, sdrplay_api_EventParamsT* params, void* cbContext) {
    std::lock_guard<std::mutex> lock(rspdxr2DeviceMapMutex);
    auto it = rspdxr2DeviceMap.find(cbContext);
    if (it != rspdxr2DeviceMap.end()) {
        RSPdxR2Control* instance = it->second;
        if (!instance) return;
        
        switch (eventId) {
            case sdrplay_api_GainChange: {
                if (instance->impl->gainCallback) {
                    auto params = instance->impl->channelParams;
                    if (params) {
                        int gRdB = params->tunerParams.gain.gRdB;
                        int lnaGRdB = params->tunerParams.gain.LNAstate;
                        float currGain = static_cast<float>(gRdB + lnaGRdB);
                        instance->impl->gainCallback->handleGainChange(gRdB, lnaGRdB, currGain);
                    }
                }
                break;
            }
            case sdrplay_api_PowerOverloadChange: {
                if (instance->impl->powerCallback) {
                    // For simplicity in this proof-of-concept implementation
                    // we'll just hard-code a value (would come from params in real API)
                    bool isOverloaded = true;
                    instance->impl->powerCallback->handlePowerOverload(isOverloaded);
                }
                break;
            }
            default:
                // Other events like DeviceRemoved, RspDuoModeChange, etc.
                break;
        }
    }
}

void RSPdxR2Control::setStreamCallback(StreamCallbackHandler* handler) {
    impl->streamCallback = handler;
}

void RSPdxR2Control::setGainCallback(GainCallbackHandler* handler) {
    impl->gainCallback = handler;
}

void RSPdxR2Control::setPowerOverloadCallback(PowerOverloadCallbackHandler* handler) {
    impl->powerCallback = handler;
}

bool RSPdxR2Control::initializeStreaming() {
    auto* device = getCurrentDevice();
    if (!device) {
        std::cerr << "No device selected for streaming" << std::endl;
        return false;
    }
    
    // Configure streaming parameters
    impl->deviceParams = getDeviceParams();
    if (!impl->deviceParams) {
        std::cerr << "Failed to get device parameters" << std::endl;
        return false;
    }
    
    impl->channelParams = impl->deviceParams->rxChannelA;
    if (!impl->channelParams) {
        std::cerr << "Failed to get channel parameters" << std::endl;
        return false;
    }
    
    // Set up default streaming parameters if not already configured
    // These are placeholders; actual values should be configurable
    if (impl->currentSampleRate <= 0) {
        setSampleRate(2.0e6); // 2 MSps default
    }
    
    if (impl->currentFreq <= 0) {
        setFrequency(100.0e6); // 100 MHz default
    }
    
    // Register device instance for callbacks
    {
        std::lock_guard<std::mutex> lock(rspdxr2DeviceMapMutex);
        rspdxr2DeviceMap[device->dev] = this;
    }
    
    // Set up callback functions
    impl->callbackFns.StreamACbFn = streamCallback;
    impl->callbackFns.StreamBCbFn = nullptr; // Not used for RSPdx
    impl->callbackFns.EventCbFn = eventCallback;
    
    return true;
}

bool RSPdxR2Control::startStreaming() {
    auto* device = getCurrentDevice();
    if (!device) {
        std::cerr << "No device selected for streaming" << std::endl;
        return false;
    }
    
    // Call the SDRPlay API to start streaming
    sdrplay_api_ErrT err = sdrplay_api_Init(device->dev, &impl->callbackFns, device->dev);
    if (err != sdrplay_api_Success) {
        std::cerr << "Failed to initialize streaming: " << sdrplay_api_GetErrorString(err) << std::endl;
        return false;
    }
    
    impl->isStreaming = true;
    return true;
}

bool RSPdxR2Control::stopStreaming() {
    auto* device = getCurrentDevice();
    if (!device) {
        return false;
    }
    
    if (!impl->isStreaming) {
        return true; // Already stopped
    }
    
    // Call the SDRPlay API to stop streaming
    sdrplay_api_ErrT err = sdrplay_api_Uninit(device->dev);
    if (err != sdrplay_api_Success) {
        std::cerr << "Failed to uninitialize streaming: " << sdrplay_api_GetErrorString(err) << std::endl;
        return false;
    }
    
    impl->isStreaming = false;
    
    // Remove device instance from callbacks map
    {
        std::lock_guard<std::mutex> lock(rspdxr2DeviceMapMutex);
        rspdxr2DeviceMap.erase(device->dev);
    }
    
    return true;
}

bool RSPdxR2Control::isStreaming() const {
    return impl->isStreaming;
}

} // namespace sdrplay
