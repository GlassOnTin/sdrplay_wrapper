// src/device_control.cpp
#include "device_control.h"
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <thread>

namespace sdrplay {

struct DeviceControl::Impl {
    sdrplay_api_DeviceT deviceStorage;
    sdrplay_api_DeviceT* device;
    sdrplay_api_DeviceParamsT* deviceParams;
    std::string lastError;

    Impl() : device(nullptr), deviceParams(nullptr) {}
};

DeviceControl::DeviceControl() : pimpl(new Impl()) {}

DeviceControl::~DeviceControl() {
    close();
}

bool DeviceControl::open() {
    sdrplay_api_ErrT err = sdrplay_api_Open();
    if (err != sdrplay_api_Success) {
        pimpl->lastError = sdrplay_api_GetErrorString(err);
        return false;
    }

    // Wait for API to initialize
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Verify API version
    float version;
    err = sdrplay_api_ApiVersion(&version);
    if (err != sdrplay_api_Success || version != SDRPLAY_API_VERSION) {
        pimpl->lastError = "API initialization failed or version mismatch";
        sdrplay_api_Close();
        return false;
    }

    return true;
}

void DeviceControl::close() {
    if (pimpl->device) {
        sdrplay_api_ReleaseDevice(pimpl->device);
        pimpl->device = nullptr;
        pimpl->deviceParams = nullptr;
    }
}

float DeviceControl::getApiVersion() const {
    float version;
    sdrplay_api_ErrT err = sdrplay_api_ApiVersion(&version);
    if (err != sdrplay_api_Success) {
        pimpl->lastError = sdrplay_api_GetErrorString(err);
        return SDRPLAY_API_VERSION;
    }
    return version;
}

std::vector<DeviceInfo> DeviceControl::getAvailableDevices() {
    std::vector<DeviceInfo> result;
    sdrplay_api_DeviceT devices[SDRPLAY_MAX_DEVICES];
    unsigned int numDevs;

    sdrplay_api_LockDeviceApi();
    sdrplay_api_ErrT err = sdrplay_api_GetDevices(devices, &numDevs, SDRPLAY_MAX_DEVICES);

    if (err == sdrplay_api_Success) {
        result.reserve(numDevs);
        for (unsigned int i = 0; i < numDevs; i++) {
            DeviceInfo info;
            info.serialNumber = devices[i].SerNo;
            info.hwVersion = devices[i].hwVer;
            info.isTunerA = (devices[i].tuner & sdrplay_api_Tuner_A) != 0;
            info.isTunerB = (devices[i].tuner & sdrplay_api_Tuner_B) != 0;
            info.isRSPDuo = (devices[i].hwVer == SDRPLAY_RSPduo_ID);
            result.push_back(info);
        }
    } else {
        pimpl->lastError = sdrplay_api_GetErrorString(err);
    }

    sdrplay_api_UnlockDeviceApi();
    return result;
}

bool DeviceControl::selectDevice(const DeviceInfo& deviceInfo) {
    sdrplay_api_DeviceT devices[SDRPLAY_MAX_DEVICES];
    unsigned int numDevs;

    sdrplay_api_LockDeviceApi();
    sdrplay_api_ErrT err = sdrplay_api_GetDevices(devices, &numDevs, SDRPLAY_MAX_DEVICES);
    if (err != sdrplay_api_Success) {
        pimpl->lastError = sdrplay_api_GetErrorString(err);
        sdrplay_api_UnlockDeviceApi();
        return false;
    }

    bool found = false;
    for (unsigned int i = 0; i < numDevs; i++) {
        if (devices[i].SerNo == deviceInfo.serialNumber) {
            err = sdrplay_api_SelectDevice(&devices[i]);
            if (err == sdrplay_api_Success) {
                pimpl->deviceStorage = devices[i];
                pimpl->device = &pimpl->deviceStorage;

                err = sdrplay_api_GetDeviceParams(pimpl->device->dev, &pimpl->deviceParams);
                if (err != sdrplay_api_Success) {
                    pimpl->lastError = sdrplay_api_GetErrorString(err);
                    pimpl->device = nullptr;
                    break;
                }
                found = true;
            } else {
                pimpl->lastError = sdrplay_api_GetErrorString(err);
            }
            break;
        }
    }

    sdrplay_api_UnlockDeviceApi();
    return found;
}

bool DeviceControl::releaseDevice() {
    if (!pimpl->device) return false;

    sdrplay_api_ErrT err = sdrplay_api_ReleaseDevice(pimpl->device);
    if (err != sdrplay_api_Success) {
        pimpl->lastError = sdrplay_api_GetErrorString(err);
        return false;
    }
    pimpl->device = nullptr;
    pimpl->deviceParams = nullptr;
    return true;
}

sdrplay_api_DeviceT* DeviceControl::getCurrentDevice() const {
    if (!pimpl->device) {
        std::cerr << "DeviceControl::getCurrentDevice - No device selected" << std::endl;
    }
    return pimpl->device;
}

sdrplay_api_DeviceParamsT* DeviceControl::getDeviceParams() const {
    return pimpl->deviceParams;
}

std::string DeviceControl::getLastError() const {
    return pimpl->lastError;
}

} // namespace sdrplay
