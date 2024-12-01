// src/sdrplay_wrapper.cpp
#include "sdrplay_wrapper.h"
#include <iostream>
#include <mutex>

namespace sdrplay {

struct Device::Impl {
    std::unique_ptr<DeviceControl> deviceControl;
    std::unique_ptr<BasicParams> basicParams;
    std::unique_ptr<ControlParams> controlParams;
    std::unique_ptr<Rsp1aParams> rsp1aParams;

    StreamCallbackHandler* pythonStreamHandler;
    GainCallbackHandler* pythonGainHandler;
    PowerOverloadCallbackHandler* pythonPowerHandler;
    std::mutex callbackMutex;

    Impl() : deviceControl(new DeviceControl()),
             pythonStreamHandler(nullptr),
             pythonGainHandler(nullptr),
             pythonPowerHandler(nullptr) {}
};

// Callback implementations
void Device::StreamACallback(short* xi, short* xq, sdrplay_api_StreamCbParamsT* params,
                         unsigned int numSamples, unsigned int reset, void* cbContext) {
    auto* device = static_cast<Device*>(cbContext);
    if (!device) return;

    std::lock_guard<std::mutex> lock(device->pimpl->callbackMutex);
    if (device->pimpl->pythonStreamHandler) {
        device->pimpl->pythonStreamHandler->handleStreamData(xi, xq, numSamples);
    }
}

void Device::EventCallback(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner,
                       sdrplay_api_EventParamsT* params, void* cbContext) {
    auto* device = static_cast<Device*>(cbContext);
    if (!device) return;

    std::lock_guard<std::mutex> lock(device->pimpl->callbackMutex);
    switch (eventId) {
        case sdrplay_api_GainChange:
            if (device->pimpl->pythonGainHandler) {
                device->pimpl->pythonGainHandler->handleGainChange(
                    params->gainParams.gRdB,
                    params->gainParams.lnaGRdB,
                    params->gainParams.currGain
                );
            }
            break;

        case sdrplay_api_PowerOverloadChange:
            if (device->pimpl->pythonPowerHandler) {
                device->pimpl->pythonPowerHandler->handlePowerOverload(
                    params->powerOverloadParams.powerOverloadChangeType ==
                    sdrplay_api_Overload_Detected
                );
            }
            break;
    }
}

// Device implementation
Device::Device() : pimpl(new Impl()) {}

Device::~Device() = default;

bool Device::open() {
    return pimpl->deviceControl->open();
}

void Device::close() {
    pimpl->deviceControl->close();
}

float Device::getApiVersion() const {
    return pimpl->deviceControl->getApiVersion();
}

std::vector<DeviceInfo> Device::getAvailableDevices() {
    return pimpl->deviceControl->getAvailableDevices();
}

bool Device::selectDevice(const DeviceInfo& device) {
    if (!pimpl->deviceControl->selectDevice(device)) {
        return false;
    }

    // Initialize parameter components after device selection
    pimpl->basicParams.reset(new BasicParams(pimpl->deviceControl.get()));
    pimpl->controlParams.reset(new ControlParams(pimpl->deviceControl.get()));
    pimpl->rsp1aParams.reset(new Rsp1aParams(pimpl->deviceControl.get()));

    // Initialize API with null callbacks since we're not streaming yet
    auto* dev = pimpl->deviceControl->getCurrentDevice();
    if (!dev) return false;

    sdrplay_api_CallbackFnsT callbacks = {};
    callbacks.StreamACbFn = nullptr;
    callbacks.StreamBCbFn = nullptr;
    callbacks.EventCbFn = nullptr;

    sdrplay_api_ErrT err = sdrplay_api_Init(dev->dev, &callbacks, this);
    if (err != sdrplay_api_Success) {
        return false;
    }

    return true;
}

bool Device::releaseDevice() {
    // Clear parameter components
    pimpl->basicParams.reset();
    pimpl->controlParams.reset();
    pimpl->rsp1aParams.reset();

    return pimpl->deviceControl->releaseDevice();
}

BasicParams* Device::getBasicParams() {
    return pimpl->basicParams.get();
}

ControlParams* Device::getControlParams() {
    return pimpl->controlParams.get();
}

Rsp1aParams* Device::getRsp1aParams() {
    return pimpl->rsp1aParams.get();
}

bool Device::startStreamingWithHandlers(
    StreamCallbackHandler* streamHandler,
    GainCallbackHandler* gainHandler,
    PowerOverloadCallbackHandler* powerHandler) {

    auto* device = pimpl->deviceControl->getCurrentDevice();
    if (!device) return false;

    pimpl->pythonStreamHandler = streamHandler;
    pimpl->pythonGainHandler = gainHandler;
    pimpl->pythonPowerHandler = powerHandler;

    sdrplay_api_CallbackFnsT callbacks;
    callbacks.StreamACbFn = &StreamACallback;
    callbacks.StreamBCbFn = nullptr;
    callbacks.EventCbFn = &EventCallback;

    sdrplay_api_ReasonForUpdateT updateReason =
        static_cast<sdrplay_api_ReasonForUpdateT>(
            sdrplay_api_Update_Ctrl_DCoffsetIQimbalance |
            sdrplay_api_Update_Ctrl_Decimation |
            sdrplay_api_Update_Ctrl_Agc
        );

    sdrplay_api_ErrT err = sdrplay_api_Update(
        device->dev,
        device->tuner,
        updateReason,
        sdrplay_api_Update_Ext1_None
    );

    if (err != sdrplay_api_Success) {
        pimpl->pythonStreamHandler = nullptr;
        pimpl->pythonGainHandler = nullptr;
        pimpl->pythonPowerHandler = nullptr;
        return false;
    }

    return true;
}

bool Device::stopStreaming() {
    auto* device = pimpl->deviceControl->getCurrentDevice();
    if (!device) return false;

    sdrplay_api_ErrT err = sdrplay_api_Uninit(device->dev);

    pimpl->pythonStreamHandler = nullptr;
    pimpl->pythonGainHandler = nullptr;
    pimpl->pythonPowerHandler = nullptr;

    return (err == sdrplay_api_Success);
}

std::string Device::getLastErrorMessage() const {
    return pimpl->deviceControl->getLastError();
}

} // namespace sdrplay
