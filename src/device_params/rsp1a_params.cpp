// src/device_params/rsp1a_params.cpp
#include "device_params/rsp1a_params.h"
#include "device_control.h"
#include "sdrplay_api.h"
#include <stdexcept>
#include <iostream>

namespace sdrplay {

struct Rsp1aParams::Impl {
    DeviceControl* deviceControl;

    Impl(DeviceControl* control) : deviceControl(control) {
        if (!control) {
            throw std::runtime_error("Invalid device control pointer");
        }
    }

    sdrplay_api_RxChannelParamsT* getChannelParams() {
        auto* deviceParams = deviceControl->getDeviceParams();
        if (!deviceParams) return nullptr;
        return deviceParams->rxChannelA;
    }

    bool isRsp1a() const {
        auto* device = deviceControl->getCurrentDevice();
        return device && (device->hwVer == SDRPLAY_RSP1A_ID ||
                         device->hwVer == SDRPLAY_RSP1B_ID);  // RSP1B uses RSP1A parameters
    }
};

Rsp1aParams::Rsp1aParams(DeviceControl* deviceControl)
    : pimpl(new Impl(deviceControl)) {}

Rsp1aParams::~Rsp1aParams() = default;

void Rsp1aParams::setBiasT(bool enable) {
    if (!pimpl->isRsp1a()) return;

    auto* channelParams = pimpl->getChannelParams();
    if (channelParams) {
        channelParams->rsp1aTunerParams.biasTEnable = enable;
    }
}

void Rsp1aParams::setRfNotch(bool enable) {
    if (!pimpl->isRsp1a()) return;

    auto* deviceParams = pimpl->deviceControl->getDeviceParams();
    if (deviceParams && deviceParams->devParams) {
        deviceParams->devParams->rsp1aParams.rfNotchEnable = enable;
    }
}

void Rsp1aParams::setDabNotch(bool enable) {
    if (!pimpl->isRsp1a()) return;

    auto* deviceParams = pimpl->deviceControl->getDeviceParams();
    if (deviceParams && deviceParams->devParams) {
        deviceParams->devParams->rsp1aParams.rfDabNotchEnable = enable;
    }
}

bool Rsp1aParams::update() {
    if (!pimpl->isRsp1a()) {
        std::cerr << "Rsp1aParams::update - Not an RSP1A device" << std::endl;
        return false;
    }

    auto* device = pimpl->deviceControl->getCurrentDevice();
    if (!device) {
        std::cerr << "Rsp1aParams::update - No current device" << std::endl;
        return false;
    }

    sdrplay_api_ReasonForUpdateT reason =
        static_cast<sdrplay_api_ReasonForUpdateT>(
            sdrplay_api_Update_Rsp1a_BiasTControl |
            sdrplay_api_Update_Rsp1a_RfNotchControl |
            sdrplay_api_Update_Rsp1a_RfDabNotchControl
        );

    sdrplay_api_ErrT err = sdrplay_api_Update(
        device->dev,
        device->tuner,
        reason,
        sdrplay_api_Update_Ext1_None
    );

    if (err != sdrplay_api_Success) {
        std::cerr << "Rsp1aParams::update - Update failed: "
                  << sdrplay_api_GetErrorString(err) << std::endl;
        return false;
    }

    return true;
}

} // namespace sdrplay
