#include "device_registry.h"
#include "device_params/rsp1a_params.h"
#include "device_params/rspdxr2_params.h"

namespace {
    // Auto-register devices at startup
    static bool registerDevices() {
        sdrplay::DeviceRegistry::registerDevice<sdrplay::RSP1AParameters>("RSP1A");
        sdrplay::DeviceRegistry::registerDevice<sdrplay::RSPdxR2Parameters>("RSPdxR2");
        return true;
    }
    static bool registered = registerDevices();
}
