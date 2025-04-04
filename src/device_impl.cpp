#include "sdrplay_wrapper.h"
#include "device_registry.h"
#include "device_control.h"
#include <vector>
#include <stdexcept>

namespace sdrplay {

// Full implementation of the Impl struct
struct Device::Impl {
    std::unique_ptr<DeviceControl> deviceControl;
    DeviceInfo currentDevice;
    bool isOpen{false};
};

} // namespace sdrplay
