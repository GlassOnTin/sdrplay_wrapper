#include "device_registry.h"
#include "device_impl/rsp1a_control.h"
#include "device_impl/rspdxr2_control.h"
#include <map>
#include <stdexcept>

namespace sdrplay {

std::map<unsigned char, DeviceControlFactory>& getFactoryMap() {
    static std::map<unsigned char, DeviceControlFactory> factories;
    return factories;
}

void DeviceRegistry::registerFactory(unsigned char hwVer, DeviceControlFactory factory) {
    getFactoryMap()[hwVer] = factory;
}

std::unique_ptr<DeviceControl> DeviceRegistry::createDeviceControl(unsigned char hwVer) {
    auto& factories = getFactoryMap();
    auto it = factories.find(hwVer);
    if (it == factories.end()) {
        throw std::runtime_error("Unsupported device hardware version");
    }
    return it->second();
}

void DeviceRegistry::clearFactories() {
    getFactoryMap().clear();
}

// Register real devices
namespace {
    struct DeviceRegistrar {
        DeviceRegistrar() {
            #ifndef SDRPLAY_TESTING
                DeviceRegistry::registerFactory(RSP1A_HWVER,
                    []() { return std::make_unique<RSP1AControl>(); });
                DeviceRegistry::registerFactory(RSPDXR2_HWVER,
                    []() { return std::make_unique<RSPdxR2Control>(); });
            #endif
        }
    };
    static DeviceRegistrar registrar;
}

} // namespace sdrplay
