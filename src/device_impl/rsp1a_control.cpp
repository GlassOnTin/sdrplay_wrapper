#include "device_impl/rsp1a_control.h"
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <cassert>
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
std::mutex deviceMapMutex;
std::map<void*, RSP1AControl*> deviceMap;

struct RSP1AControl::RSP1AImpl {
    sdrplay_api_DeviceParamsT* deviceParams{nullptr};
    sdrplay_api_RxChannelParamsT* channelParams{nullptr};
    double currentFreq{100.0e6};
    double currentSampleRate{2.0e6};
    int currentGain{40};
    int currentLnaState{0};
    bool isStreaming{false};
    
    // Callback handlers
    StreamCallbackHandler* streamCallback{nullptr};
    GainCallbackHandler* gainCallback{nullptr};
    PowerOverloadCallbackHandler* powerCallback{nullptr};
    
    // SDRPlay API callback functions
    sdrplay_api_CallbackFnsT callbackFns;

    void updateChannel() {
        if (deviceParams) {
            channelParams = deviceParams->rxChannelA;
        }
    }
};

RSP1AControl::RSP1AControl() : impl(std::make_unique<RSP1AImpl>()) {}

RSP1AControl::~RSP1AControl() = default;

void RSP1AControl::setFrequency(double freq) {
    impl->deviceParams = getDeviceParams();
    impl->updateChannel();

    if (impl->channelParams) {
        impl->channelParams->tunerParams.rfFreq.rfHz = freq;
        impl->currentFreq = freq;

        auto* device = getCurrentDevice();
        if (device) {
            sdrplay_api_Update(device->dev, device->tuner,
                             sdrplay_api_Update_Tuner_Frf,
                             sdrplay_api_Update_Ext1_None);
        }
    }
}

double RSP1AControl::getFrequency() const {
    impl->deviceParams = getDeviceParams();
    impl->updateChannel();

    if (impl->channelParams) {
        return impl->channelParams->tunerParams.rfFreq.rfHz;
    }
    return impl->currentFreq;
}

void RSP1AControl::setSampleRate(double rate) {
    impl->deviceParams = getDeviceParams();

    if (impl->deviceParams && impl->deviceParams->devParams) {
        impl->deviceParams->devParams->fsFreq.fsHz = rate;
        impl->currentSampleRate = rate;

        auto* device = getCurrentDevice();
        if (device) {
            sdrplay_api_Update(device->dev, device->tuner,
                             sdrplay_api_Update_Dev_Fs,
                             sdrplay_api_Update_Ext1_None);
        }
    }
}

double RSP1AControl::getSampleRate() const {
    impl->deviceParams = getDeviceParams();

    if (impl->deviceParams && impl->deviceParams->devParams) {
        return impl->deviceParams->devParams->fsFreq.fsHz;
    }
    return impl->currentSampleRate;
}

void RSP1AControl::setGainReduction(int gain) {
    impl->deviceParams = getDeviceParams();
    impl->updateChannel();

    if (impl->channelParams) {
        impl->channelParams->tunerParams.gain.gRdB = gain;
        impl->currentGain = gain;

        auto* device = getCurrentDevice();
        if (device) {
            sdrplay_api_Update(device->dev, device->tuner,
                             sdrplay_api_Update_Tuner_Gr,
                             sdrplay_api_Update_Ext1_None);
        }
    }
}

void RSP1AControl::setLNAState(int state) {
    impl->deviceParams = getDeviceParams();
    impl->updateChannel();

    if (impl->channelParams) {
        impl->channelParams->tunerParams.gain.LNAstate = state;
        impl->currentLnaState = state;

        auto* device = getCurrentDevice();
        if (device) {
            sdrplay_api_Update(device->dev, device->tuner,
                             sdrplay_api_Update_Tuner_Gr,
                             sdrplay_api_Update_Ext1_None);
        }
    }
}

// Static callback implementations
void RSP1AControl::streamCallback(short* xi, short* xq, sdrplay_api_StreamCbParamsT* params, 
                                 unsigned int numSamples, unsigned int reset, void* cbContext) {
    std::lock_guard<std::mutex> lock(deviceMapMutex);
    auto it = deviceMap.find(cbContext);
    if (it != deviceMap.end()) {
        RSP1AControl* instance = it->second;
        if (instance && instance->impl->streamCallback) {
            instance->impl->streamCallback->handleStreamData(xi, xq, numSamples);
        }
    }
}

void RSP1AControl::eventCallback(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner, sdrplay_api_EventParamsT* params, void* cbContext) {
    std::lock_guard<std::mutex> lock(deviceMapMutex);
    auto it = deviceMap.find(cbContext);
    if (it != deviceMap.end()) {
        RSP1AControl* instance = it->second;
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

void RSP1AControl::setStreamCallback(StreamCallbackHandler* handler) {
    impl->streamCallback = handler;
}

void RSP1AControl::setGainCallback(GainCallbackHandler* handler) {
    impl->gainCallback = handler;
}

void RSP1AControl::setPowerOverloadCallback(PowerOverloadCallbackHandler* handler) {
    impl->powerCallback = handler;
}

bool RSP1AControl::initializeStreaming() {
    auto* device = getCurrentDevice();
    if (!device) {
        std::cerr << "No device selected for streaming" << std::endl;
        return false;
    }
    
    // Configure streaming parameters
    impl->deviceParams = getDeviceParams();
    impl->updateChannel();
    if (!impl->deviceParams || !impl->channelParams) {
        std::cerr << "Failed to get device parameters" << std::endl;
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
        std::lock_guard<std::mutex> lock(deviceMapMutex);
        deviceMap[device->dev] = this;
    }
    
    // Set up callback functions
    impl->callbackFns.StreamACbFn = streamCallback;
    impl->callbackFns.StreamBCbFn = nullptr; // Not used for RSP1A
    impl->callbackFns.EventCbFn = eventCallback;
    
    return true;
}

bool RSP1AControl::startStreaming() {
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

bool RSP1AControl::stopStreaming() {
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
        std::lock_guard<std::mutex> lock(deviceMapMutex);
        deviceMap.erase(device->dev);
    }
    
    return true;
}

bool RSP1AControl::isStreaming() const {
    return impl->isStreaming;
}

} // namespace sdrplay
