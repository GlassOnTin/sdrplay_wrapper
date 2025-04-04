#pragma once
#include "device_control.h"
#include <memory>
#include <functional>

namespace sdrplay {

using DeviceControlFactory = std::function<std::unique_ptr<DeviceControl>()>;

class DeviceRegistry {
public:
    static std::unique_ptr<DeviceControl> createDeviceControl(unsigned char hwVer);
    static void registerFactory(unsigned char hwVer, DeviceControlFactory factory);
    static void clearFactories();

private:
    DeviceRegistry() = delete;
};

} // namespace sdrplay
