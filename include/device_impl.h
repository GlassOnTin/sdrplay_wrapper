// include/device_impl.h
#pragma once
#include "sdrplay_wrapper.h"
#include "device_control.h"
#include "device_params/rsp1a_params.h"
#include "device_params/rspdxr2_params.h"
#include <memory>

namespace sdrplay {

class StreamCallbackHandler;
class GainCallbackHandler;
class PowerOverloadCallbackHandler;

struct Device::Impl {
    std::unique_ptr<DeviceControl> deviceControl;
    DeviceInfo currentDevice;
    bool isOpen{false};
    bool streaming{false};
    
    // Device-specific parameter objects
    std::unique_ptr<Rsp1aParams> rsp1aParams;
    std::unique_ptr<RspDxR2Params> rspdxr2Params;

    // Callback handlers
    StreamCallbackHandler* streamCallback{nullptr};
    GainCallbackHandler* gainCallback{nullptr};
    PowerOverloadCallbackHandler* powerCallback{nullptr};

    Impl() : deviceControl(nullptr), isOpen(false), streaming(false),
             streamCallback(nullptr), gainCallback(nullptr), powerCallback(nullptr) {}
};

} // namespace sdrplay
