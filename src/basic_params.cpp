// src/basic_params.cpp
#include "basic_params.h"
#include "device_control.h"
#include "sdrplay_api.h"
#include <stdexcept>
#include <iostream>

namespace sdrplay {

struct BasicParams::Impl {
    DeviceControl* deviceControl;

    Impl(DeviceControl* control) : deviceControl(control) {
        if (!control) {
            throw std::runtime_error("Invalid device control pointer");
        }
    }

    sdrplay_api_RxChannelParamsT* getChannelParams() {
        auto* deviceParams = deviceControl->getDeviceParams();
        if (!deviceParams) return nullptr;
        return deviceParams->rxChannelA; // Default to channel A
    }
};

BasicParams::BasicParams(DeviceControl* deviceControl)
    : pimpl(new Impl(deviceControl)) {}

BasicParams::~BasicParams() = default;

void BasicParams::setSampleRate(double sampleRateHz) {
    auto* deviceParams = pimpl->deviceControl->getDeviceParams();
    if (deviceParams && deviceParams->devParams) {
        deviceParams->devParams->fsFreq.fsHz = sampleRateHz;
    }
}

void BasicParams::setRfFrequency(double frequencyHz) {
    auto* channelParams = pimpl->getChannelParams();
    if (channelParams) {
        channelParams->tunerParams.rfFreq.rfHz = frequencyHz;
    }
}

void BasicParams::setBandwidth(int bandwidthKHz) {
    auto* channelParams = pimpl->getChannelParams();
    if (channelParams) {
        switch (bandwidthKHz) {
            case 200: channelParams->tunerParams.bwType = sdrplay_api_BW_0_200; break;
            case 300: channelParams->tunerParams.bwType = sdrplay_api_BW_0_300; break;
            case 600: channelParams->tunerParams.bwType = sdrplay_api_BW_0_600; break;
            case 1536: channelParams->tunerParams.bwType = sdrplay_api_BW_1_536; break;
            case 5000: channelParams->tunerParams.bwType = sdrplay_api_BW_5_000; break;
            case 6000: channelParams->tunerParams.bwType = sdrplay_api_BW_6_000; break;
            case 7000: channelParams->tunerParams.bwType = sdrplay_api_BW_7_000; break;
            case 8000: channelParams->tunerParams.bwType = sdrplay_api_BW_8_000; break;
            default: channelParams->tunerParams.bwType = sdrplay_api_BW_0_200; break;
        }
    }
}

void BasicParams::setIfType(int ifkHz) {
    auto* channelParams = pimpl->getChannelParams();
    if (channelParams) {
        switch (ifkHz) {
            case 0: channelParams->tunerParams.ifType = sdrplay_api_IF_Zero; break;
            case 450: channelParams->tunerParams.ifType = sdrplay_api_IF_0_450; break;
            case 1620: channelParams->tunerParams.ifType = sdrplay_api_IF_1_620; break;
            case 2048: channelParams->tunerParams.ifType = sdrplay_api_IF_2_048; break;
            default: channelParams->tunerParams.ifType = sdrplay_api_IF_Zero; break;
        }
    }
}

void BasicParams::setGain(int gainReduction, int lnaState) {
    auto* channelParams = pimpl->getChannelParams();
    if (channelParams) {
        channelParams->tunerParams.gain.gRdB = gainReduction;
        channelParams->tunerParams.gain.LNAstate = lnaState;
    }
}

bool BasicParams::update() {
    auto* device = pimpl->deviceControl->getCurrentDevice();
    if (!device) {
        std::cerr << "BasicParams::update - No current device" << std::endl;
        return false;
    }

    sdrplay_api_ReasonForUpdateT reason =
        static_cast<sdrplay_api_ReasonForUpdateT>(
            sdrplay_api_Update_Dev_Fs |
            sdrplay_api_Update_Tuner_Frf |
            sdrplay_api_Update_Tuner_BwType |
            sdrplay_api_Update_Tuner_IfType |
            sdrplay_api_Update_Tuner_Gr
        );

    sdrplay_api_ErrT err = sdrplay_api_Update(
        device->dev,
        device->tuner,
        reason,
        sdrplay_api_Update_Ext1_None
    );

    if (err != sdrplay_api_Success) {
        std::cerr << "BasicParams::update - Update failed: "
                  << sdrplay_api_GetErrorString(err) << std::endl;
        return false;
    }

    return true;
}

} // namespace sdrplay
