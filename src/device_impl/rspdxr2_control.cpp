#include "device_impl/rspdxr2_control.h"
#include <stdexcept>
#include <iostream>

namespace sdrplay {

struct RSPdxR2Control::RSPdxR2Impl {
    sdrplay_api_DeviceParamsT* deviceParams{nullptr};
    sdrplay_api_RxChannelParamsT* channelParams{nullptr};
    double currentFreq{100.0e6};
    double currentSampleRate{2.0e6};
    bool hdrMode{false};
    bool biasTEnabled{false};
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

} // namespace sdrplay
