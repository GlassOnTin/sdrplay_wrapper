#include "device_impl/rsp1a_control.h"
#include "sdrplay_exception.h"
#include <iostream>
#include <mutex>
#include <cassert>
#include <map>

namespace sdrplay {

struct RSP1AControl::RSP1AImpl {
    sdrplay_api_DeviceParamsT* deviceParams{nullptr};
    sdrplay_api_RxChannelParamsT* channelParams{nullptr};
    double currentFreq{100.0e6};
    double currentSampleRate{2.0e6};
    int currentGain{40};
    int currentLnaState{0};

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


} // namespace sdrplay
