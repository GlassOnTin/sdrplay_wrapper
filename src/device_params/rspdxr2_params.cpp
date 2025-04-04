// src/device_params/rspdxr2_params.cpp
#include "device_params/rspdxr2_params.h"
#include "device_control.h"
#include "sdrplay_api.h"
#include <stdexcept>
#include <iostream>

namespace sdrplay {

struct RspDxR2Params::Impl {
    DeviceControl* deviceControl;

    Impl(DeviceControl* control) : deviceControl(control) {
        if (!control) {
            throw std::runtime_error("Invalid device control pointer");
        }
    }

    bool isRspDxR2() const {
        auto* device = deviceControl->getCurrentDevice();
        return device && (device->hwVer == SDRPLAY_RSPdxR2_ID);
    }

    sdrplay_api_RxChannelParamsT* getChannelParams() {
        auto* deviceParams = deviceControl->getDeviceParams();
        if (!deviceParams) return nullptr;
        return deviceParams->rxChannelA;
    }
};

RspDxR2Params::RspDxR2Params(DeviceControl* deviceControl)
    : pimpl(new Impl(deviceControl)) {}

RspDxR2Params::~RspDxR2Params() = default;

bool RspDxR2Params::update() {
    // Since RSPdxR2 uses same parameters as RSPdx, we just need to verify
    // device type and parameters are valid
    if (!pimpl->isRspDxR2()) {
        std::cerr << "RspDxR2Params::update - Not an RSPdxR2 device" << std::endl;
        return false;
    }

    auto* device = pimpl->deviceControl->getCurrentDevice();
    if (!device) {
        std::cerr << "RspDxR2Params::update - No current device" << std::endl;
        return false;
    }

    // RSPdxR2 uses same parameter updates as RSPdx
    sdrplay_api_ErrT err = sdrplay_api_Update(
        device->dev,
        device->tuner,
        static_cast<sdrplay_api_ReasonForUpdateT>(0),  // No unique RSPdxR2 parameters to update
        sdrplay_api_Update_Ext1_None
    );

    if (err != sdrplay_api_Success) {
        std::cerr << "RspDxR2Params::update - Update failed: "
                  << sdrplay_api_GetErrorString(err) << std::endl;
        return false;
    }

    return true;
}

} // namespace sdrplay
