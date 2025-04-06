// include/device_impl.h
#pragma once
#include "sdrplay_wrapper.h"
#include "device_control.h"
#include "device_params/rsp1a_params.h"
#include "device_params/rspdxr2_params.h"
#include <memory>

namespace sdrplay {

struct Device::Impl {
    std::unique_ptr<DeviceControl> deviceControl;
    DeviceInfo currentDevice;
    bool isOpen{false};
    
    // Device-specific parameter objects
    std::unique_ptr<Rsp1aParams> rsp1aParams;
    std::unique_ptr<RspDxR2Params> rspdxr2Params;

    Impl() : deviceControl(nullptr), isOpen(false) {}
};

} // namespace sdrplay
