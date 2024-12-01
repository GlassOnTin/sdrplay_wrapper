// src/control_params.cpp
#include "control_params.h"
#include "device_control.h"
#include "sdrplay_api.h"
#include <stdexcept>
#include <iostream>

namespace sdrplay {

struct ControlParams::Impl {
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

ControlParams::ControlParams(DeviceControl* deviceControl)
    : pimpl(new Impl(deviceControl)) {}

ControlParams::~ControlParams() = default;

void ControlParams::setAgcControl(bool enable, int setPoint) {
    auto* channelParams = pimpl->getChannelParams();
    if (channelParams) {
        channelParams->ctrlParams.agc.enable = enable ? sdrplay_api_AGC_CTRL_EN : sdrplay_api_AGC_DISABLE;
        channelParams->ctrlParams.agc.setPoint_dBfs = setPoint;
    }
}

void ControlParams::setDcOffset(bool dcEnable, bool iqEnable) {
    auto* channelParams = pimpl->getChannelParams();
    if (channelParams) {
        channelParams->ctrlParams.dcOffset.DCenable = dcEnable;
        channelParams->ctrlParams.dcOffset.IQenable = iqEnable;
    }
}

void ControlParams::setDecimation(bool enable, unsigned char decimationFactor, bool wideBandSignal) {
    auto* channelParams = pimpl->getChannelParams();
    if (channelParams) {
        channelParams->ctrlParams.decimation.enable = enable;
        channelParams->ctrlParams.decimation.decimationFactor = decimationFactor;
        channelParams->ctrlParams.decimation.wideBandSignal = wideBandSignal;
    }
}

bool ControlParams::update() {
    auto* device = pimpl->deviceControl->getCurrentDevice();
    if (!device) {
        std::cerr << "ControlParams::update - No current device" << std::endl;
        return false;
    }

    sdrplay_api_ReasonForUpdateT reason =
        static_cast<sdrplay_api_ReasonForUpdateT>(
            sdrplay_api_Update_Ctrl_DCoffsetIQimbalance |
            sdrplay_api_Update_Ctrl_Decimation |
            sdrplay_api_Update_Ctrl_Agc
        );

    sdrplay_api_ErrT err = sdrplay_api_Update(
        device->dev,
        device->tuner,
        reason,
        sdrplay_api_Update_Ext1_None
    );

    if (err != sdrplay_api_Success) {
        std::cerr << "ControlParams::update - Update failed: "
                  << sdrplay_api_GetErrorString(err) << std::endl;
        return false;
    }

    return true;
}

} // namespace sdrplay
