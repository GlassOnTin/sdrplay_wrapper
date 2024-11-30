// sdrplay_wrapper.cpp
#include "sdrplay_wrapper.h"
#include "sdrplay_api.h"
#include <mutex>
#include <stdexcept>

namespace sdrplay {

// Private implementation structures
struct Device::Impl {
    sdrplay_api_DeviceT* device = nullptr;
    sdrplay_api_DeviceParamsT* deviceParams = nullptr;
    StreamCallback streamCallback;
    GainCallback gainCallback;
    PowerOverloadCallback powerCallback;
    std::mutex callbackMutex;
    std::string lastError;
};

struct DeviceParams::Impl {
    Device* device;
    sdrplay_api_DeviceParamsT* params;
};

struct RxChannelParams::Impl {
    Device* device;
    sdrplay_api_RxChannelParamsT* params;
    bool isTunerB;
};

// Callback handlers
static void StreamACallback(short* xi, short* xq, sdrplay_api_StreamCbParamsT* params,
                          unsigned int numSamples, unsigned int reset, void* cbContext) {
    auto* device = static_cast<Device::Impl*>(cbContext);
    std::lock_guard<std::mutex> lock(device->callbackMutex);
    if (device->streamCallback) {
        device->streamCallback(xi, xq, numSamples);
    }
}

static void EventCallback(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner,
                         sdrplay_api_EventParamsT* params, void* cbContext) {
    auto* device = static_cast<Device::Impl*>(cbContext);
    std::lock_guard<std::mutex> lock(device->callbackMutex);

    switch (eventId) {
        case sdrplay_api_GainChange:
            if (device->gainCallback) {
                device->gainCallback(
                    params->gainParams.gRdB,
                    params->gainParams.lnaGRdB,
                    params->gainParams.currGain
                );
            }
            break;

        case sdrplay_api_PowerOverloadChange:
            if (device->powerCallback) {
                device->powerCallback(
                    params->powerOverloadParams.powerOverloadChangeType ==
                    sdrplay_api_Overload_Detected
                );
            }
            break;
    }
}

// Device implementation
Device::Device() : pimpl(new Impl()) {}

Device::~Device() {
    close();
}

bool Device::open() {
    sdrplay_api_ErrT err = sdrplay_api_Open();
    if (err != sdrplay_api_Success) {
        pimpl->lastError = sdrplay_api_GetErrorString(err);
        return false;
    }
    return true;
}

void Device::close() {
    if (pimpl->device) {
        stopStreaming();
        sdrplay_api_ReleaseDevice(pimpl->device);
        pimpl->device = nullptr;
    }
    sdrplay_api_Close();
}

float Device::getApiVersion() const {
    float version;
    sdrplay_api_ApiVersion(&version);
    return version;
}

std::vector<Device::DeviceInfo> Device::getAvailableDevices() {
    std::vector<DeviceInfo> result;
    sdrplay_api_DeviceT devices[SDRPLAY_MAX_DEVICES];
    unsigned int numDevs;

    sdrplay_api_LockDeviceApi();
    sdrplay_api_ErrT err = sdrplay_api_GetDevices(devices, &numDevs, SDRPLAY_MAX_DEVICES);
    if (err == sdrplay_api_Success) {
        for (unsigned int i = 0; i < numDevs; i++) {
            DeviceInfo info;
            info.serialNumber = devices[i].SerNo;
            info.hwVersion = devices[i].hwVer;
            info.isTunerA = (devices[i].tuner & sdrplay_api_Tuner_A) != 0;
            info.isTunerB = (devices[i].tuner & sdrplay_api_Tuner_B) != 0;
            info.isRSPDuo = (devices[i].hwVer == SDRPLAY_RSPduo_ID);
            result.push_back(info);
        }
    }
    sdrplay_api_UnlockDeviceApi();
    return result;
}

bool Device::selectDevice(const DeviceInfo& deviceInfo) {
    sdrplay_api_DeviceT devices[SDRPLAY_MAX_DEVICES];
    unsigned int numDevs;

    sdrplay_api_LockDeviceApi();
    sdrplay_api_ErrT err = sdrplay_api_GetDevices(devices, &numDevs, SDRPLAY_MAX_DEVICES);
    if (err != sdrplay_api_Success) {
        sdrplay_api_UnlockDeviceApi();
        return false;
    }

    // Find matching device
    for (unsigned int i = 0; i < numDevs; i++) {
        if (devices[i].SerNo == deviceInfo.serialNumber) {
            err = sdrplay_api_SelectDevice(&devices[i]);
            if (err == sdrplay_api_Success) {
                pimpl->device = &devices[i];
                sdrplay_api_GetDeviceParams(pimpl->device->dev, &pimpl->deviceParams);
            }
            break;
        }
    }

    sdrplay_api_UnlockDeviceApi();
    return (err == sdrplay_api_Success);
}

bool Device::startStreaming(StreamCallback streamCb, GainCallback gainCb, PowerOverloadCallback powerCb) {
    if (!pimpl->device) return false;

    pimpl->streamCallback = streamCb;
    pimpl->gainCallback = gainCb;
    pimpl->powerCallback = powerCb;

    sdrplay_api_CallbackFnsT callbacks;
    callbacks.StreamACbFn = StreamACallback;
    callbacks.StreamBCbFn = nullptr;
    callbacks.EventCbFn = EventCallback;

    sdrplay_api_ErrT err = sdrplay_api_Init(pimpl->device->dev, &callbacks, pimpl.get());
    return (err == sdrplay_api_Success);
}

bool Device::stopStreaming() {
    if (!pimpl->device) return false;

    sdrplay_api_ErrT err = sdrplay_api_Uninit(pimpl->device->dev);
    return (err == sdrplay_api_Success);
}

std::string Device::getLastErrorMessage() const {
    return pimpl->lastError;
}

// Parameter class implementations
DeviceParams::DeviceParams(Device* device) : pimpl(new Impl{device, device->pimpl->deviceParams}) {}

void DeviceParams::setSampleRate(double sampleRateHz) {
    if (pimpl->params && pimpl->params->devParams) {
        pimpl->params->devParams->fsFreq.fsHz = sampleRateHz;
    }
}

bool DeviceParams::update() {
    if (!pimpl->device->pimpl->device) return false;

    sdrplay_api_ErrT err = sdrplay_api_Update(
        pimpl->device->pimpl->device->dev,
        pimpl->device->pimpl->device->tuner,
        sdrplay_api_Update_Dev_Fs,
        sdrplay_api_Update_Ext1_None
    );
    return (err == sdrplay_api_Success);
}

RxChannelParams::RxChannelParams(Device* device, bool isTunerB)
    : pimpl(new Impl{device, nullptr, isTunerB}) {
    if (device->pimpl->deviceParams) {
        pimpl->params = isTunerB ? device->pimpl->deviceParams->rxChannelB
                                : device->pimpl->deviceParams->rxChannelA;
    }
}

void RxChannelParams::setRfFrequency(double frequencyHz) {
    if (pimpl->params) {
        pimpl->params->tunerParams.rfFreq.rfHz = frequencyHz;
    }
}

bool RxChannelParams::update() {
    if (!pimpl->device->pimpl->device) return false;

    sdrplay_api_ErrT err = sdrplay_api_Update(
        pimpl->device->pimpl->device->dev,
        pimpl->isTunerB ? sdrplay_api_Tuner_B : sdrplay_api_Tuner_A,
        sdrplay_api_Update_Tuner_Frf,
        sdrplay_api_Update_Ext1_None
    );
    return (err == sdrplay_api_Success);
}

} // namespace sdrplay
