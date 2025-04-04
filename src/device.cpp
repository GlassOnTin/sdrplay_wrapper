// src/device.cpp
#include "sdrplay_wrapper.h"
#include "device_impl.h"
#include "device_registry.h"
#include "device_types.h"
#include "device_params/rsp1a_params.h"
#include "device_params/rspdxr2_params.h"
#include <vector>
#include <string>
#include <stdexcept>

namespace sdrplay {

Device::Device() : pimpl(new Impl()) {}
Device::~Device() = default;

bool Device::selectDevice(const DeviceInfo& deviceInfo) {
    try {
        auto control = DeviceRegistry::createDeviceControl(deviceInfo.hwVer);
        if (!control) {
            return false;
        }

        // Initialize the device
        if (!control->open()) {
            return false;
        }

        pimpl->currentDevice = deviceInfo;
        pimpl->deviceControl = std::move(control);
        pimpl->isOpen = true;

        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool Device::releaseDevice() {
    if (pimpl->deviceControl) {
        pimpl->deviceControl->close();
        pimpl->deviceControl.reset();
    }
    pimpl->isOpen = false;
    return true;
}

std::vector<DeviceInfo> Device::getAvailableDevices() {
    auto control = DeviceRegistry::createDeviceControl(RSP1A_HWVER);  // Using constant from device_types.h
    if (!control) {
        return std::vector<DeviceInfo>();
    }
    return control->getAvailableDevices();
}

void Device::setFrequency(double freq) {
    if (pimpl->deviceControl) {
        pimpl->deviceControl->setFrequency(freq);
    }
}

double Device::getFrequency() const {
    return pimpl->deviceControl ? pimpl->deviceControl->getFrequency() : 0.0;
}

void Device::setSampleRate(double rate) {
    if (pimpl->deviceControl) {
        pimpl->deviceControl->setSampleRate(rate);
    }
}

double Device::getSampleRate() const {
    return pimpl->deviceControl ? pimpl->deviceControl->getSampleRate() : 0.0;
}

Rsp1aParams* Device::getRsp1aParams() {
    if (!pimpl->deviceControl || pimpl->currentDevice.hwVer != RSP1A_HWVER) {
        return nullptr;
    }
    
    if (!pimpl->rsp1aParams) {
        pimpl->rsp1aParams = std::make_unique<Rsp1aParams>(pimpl->deviceControl.get());
    }
    
    return pimpl->rsp1aParams.get();
}

RspDxR2Params* Device::getRspDxR2Params() {
    if (!pimpl->deviceControl || pimpl->currentDevice.hwVer != RSPDXR2_HWVER) {
        return nullptr;
    }
    
    if (!pimpl->rspdxr2Params) {
        pimpl->rspdxr2Params = std::make_unique<RspDxR2Params>(pimpl->deviceControl.get());
    }
    
    return pimpl->rspdxr2Params.get();
}

} // namespace sdrplay
