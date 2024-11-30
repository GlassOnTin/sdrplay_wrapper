// sdrplay_wrapper.cpp
#include "sdrplay_wrapper.h"
#include <stdexcept>

namespace sdrplay {
namespace detail {

void StreamACallback(short* xi, short* xq, sdrplay_api_StreamCbParamsT* params,
                    unsigned int numSamples, unsigned int reset, void* cbContext) {
    auto* device = static_cast<Device*>(cbContext);
    std::lock_guard<std::mutex> lock(device->pimpl->callbackMutex);
    if (device->pimpl->streamCallback) {
        device->pimpl->streamCallback(xi, xq, numSamples);
    }
}

void EventCallback(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner,
                  sdrplay_api_EventParamsT* params, void* cbContext) {
    auto* device = static_cast<Device*>(cbContext);
    std::lock_guard<std::mutex> lock(device->pimpl->callbackMutex);

    switch (eventId) {
        case sdrplay_api_GainChange:
            if (device->pimpl->gainCallback) {
                device->pimpl->gainCallback(
                    params->gainParams.gRdB,
                    params->gainParams.lnaGRdB,
                    params->gainParams.currGain
                );
            }
            break;

        case sdrplay_api_PowerOverloadChange:
            if (device->pimpl->powerCallback) {
                device->pimpl->powerCallback(
                    params->powerOverloadParams.powerOverloadChangeType ==
                    sdrplay_api_Overload_Detected
                );
            }
            break;
    }
}

} // namespace detail

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
    //sdrplay_api_Close();
}

bool Device::releaseDevice() {
    if (!pimpl->device) return false;

    sdrplay_api_ErrT err = sdrplay_api_ReleaseDevice(pimpl->device);
    if (err != sdrplay_api_Success) {
        pimpl->lastError = sdrplay_api_GetErrorString(err);
        return false;
    }
    pimpl->device = nullptr;
    return true;
}

float Device::getApiVersion() const {
    float version;
    sdrplay_api_ApiVersion(&version);
    return version;
}

DeviceParams* Device::getDeviceParams() {
    if (!pimpl->device) return nullptr;
    return new DeviceParams(this);
}

RxChannelParams* Device::getRxChannelParams(bool isTunerB) {
    if (!pimpl->device) return nullptr;
    return new RxChannelParams(this, isTunerB);
}

std::vector<DeviceInfo> Device::getAvailableDevices() {
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
    callbacks.StreamACbFn = &detail::StreamACallback;
    callbacks.StreamBCbFn = nullptr;
    callbacks.EventCbFn = &detail::EventCallback;

    sdrplay_api_ErrT err = sdrplay_api_Init(pimpl->device->dev, &callbacks, this);
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

// DeviceParams implementation
DeviceParams::DeviceParams(Device* device)
    : pimpl(new Impl(device, device->pimpl->deviceParams)) {}

void DeviceParams::setSampleRate(double sampleRateHz) {
    if (pimpl->params && pimpl->params->devParams) {
        pimpl->params->devParams->fsFreq.fsHz = sampleRateHz;
    }
}

void DeviceParams::setPpm(double ppm) {
    if (pimpl->params && pimpl->params->devParams) {
        pimpl->params->devParams->ppm = ppm;
    }
}

bool DeviceParams::update() {
    if (!pimpl->device->pimpl->device) return false;

    sdrplay_api_ReasonForUpdateT reason =
        static_cast<sdrplay_api_ReasonForUpdateT>(
            sdrplay_api_Update_Dev_Fs |
            sdrplay_api_Update_Dev_Ppm
        );

    sdrplay_api_ErrT err = sdrplay_api_Update(
        pimpl->device->pimpl->device->dev,
        pimpl->device->pimpl->device->tuner,
        reason,
        sdrplay_api_Update_Ext1_None
    );
    return (err == sdrplay_api_Success);
}

// RxChannelParams implementation
RxChannelParams::RxChannelParams(Device* device, bool isTunerB)
    : pimpl(new Impl(device, nullptr, isTunerB)) {
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

void RxChannelParams::setBandwidth(int bandwidthKHz) {
    if (pimpl->params) {
        switch (bandwidthKHz) {
            case 200: pimpl->params->tunerParams.bwType = sdrplay_api_BW_0_200; break;
            case 300: pimpl->params->tunerParams.bwType = sdrplay_api_BW_0_300; break;
            case 600: pimpl->params->tunerParams.bwType = sdrplay_api_BW_0_600; break;
            case 1536: pimpl->params->tunerParams.bwType = sdrplay_api_BW_1_536; break;
            case 5000: pimpl->params->tunerParams.bwType = sdrplay_api_BW_5_000; break;
            case 6000: pimpl->params->tunerParams.bwType = sdrplay_api_BW_6_000; break;
            case 7000: pimpl->params->tunerParams.bwType = sdrplay_api_BW_7_000; break;
            case 8000: pimpl->params->tunerParams.bwType = sdrplay_api_BW_8_000; break;
            default: pimpl->params->tunerParams.bwType = sdrplay_api_BW_0_200; break;
        }
    }
}

void RxChannelParams::setIFType(int ifkHz) {
    if (pimpl->params) {
        switch (ifkHz) {
            case 0: pimpl->params->tunerParams.ifType = sdrplay_api_IF_Zero; break;
            case 450: pimpl->params->tunerParams.ifType = sdrplay_api_IF_0_450; break;
            case 1620: pimpl->params->tunerParams.ifType = sdrplay_api_IF_1_620; break;
            case 2048: pimpl->params->tunerParams.ifType = sdrplay_api_IF_2_048; break;
            default: pimpl->params->tunerParams.ifType = sdrplay_api_IF_Zero; break;
        }
    }
}

void RxChannelParams::setGain(int gainReduction, int lnaState) {
    if (pimpl->params) {
        pimpl->params->tunerParams.gain.gRdB = gainReduction;
        pimpl->params->tunerParams.gain.LNAstate = lnaState;
    }
}

void RxChannelParams::setAgcControl(bool enable, int setPoint) {
    if (pimpl->params) {
        pimpl->params->ctrlParams.agc.enable = enable ? sdrplay_api_AGC_CTRL_EN : sdrplay_api_AGC_DISABLE;
        pimpl->params->ctrlParams.agc.setPoint_dBfs = setPoint;
    }
}

bool RxChannelParams::update() {
    if (!pimpl->device->pimpl->device) return false;

    sdrplay_api_ReasonForUpdateT reason =
        static_cast<sdrplay_api_ReasonForUpdateT>(
            sdrplay_api_Update_Tuner_Frf |
            sdrplay_api_Update_Tuner_BwType |
            sdrplay_api_Update_Tuner_IfType |
            sdrplay_api_Update_Tuner_Gr |
            sdrplay_api_Update_Ctrl_Agc
        );

    sdrplay_api_ErrT err = sdrplay_api_Update(
        pimpl->device->pimpl->device->dev,
        pimpl->isTunerB ? sdrplay_api_Tuner_B : sdrplay_api_Tuner_A,
        reason,
        sdrplay_api_Update_Ext1_None
    );
    return (err == sdrplay_api_Success);
}

} // namespace sdrplay
