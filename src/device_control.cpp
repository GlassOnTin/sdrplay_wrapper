#include "device_control.h"
#include "sdrplay_exception.h"
#include <cstring>
#include <iostream>

namespace sdrplay {

struct DeviceControl::Impl {
    sdrplay_api_DeviceT* currentDevice{nullptr};
    sdrplay_api_DeviceParamsT* deviceParams{nullptr};
    std::string lastError;
};

DeviceControl::DeviceControl() : impl(std::make_unique<Impl>()) {}

DeviceControl::~DeviceControl() {
    close();
}

bool DeviceControl::open() {
    sdrplay_api_ErrT err = sdrplay_api_Open();
    if (err != sdrplay_api_Success) {
        impl->lastError = sdrplay_api_GetErrorString(err);
        return false;
    }
    return true;
}

void DeviceControl::close() {
    if (impl->currentDevice) {
        releaseDevice();
        sdrplay_api_Close();
        impl->currentDevice = nullptr;
        impl->deviceParams = nullptr;
    }
}

float DeviceControl::getApiVersion() const {
    float version = 0.0f;
    sdrplay_api_ApiVersion(&version);
    return version;
}

std::vector<DeviceInfo> DeviceControl::getAvailableDevices() {
    std::vector<DeviceInfo> result;
    
    // Make sure the API is opened
    if (open()) {
        std::cout << "API successfully opened" << std::endl;
    } else {
        std::cerr << "Failed to open API: " << impl->lastError << std::endl;
        return result;
    }

    sdrplay_api_DeviceT devices[SDRPLAY_MAX_DEVICES];
    unsigned int numDevs = 0;

    std::cout << "Getting device list..." << std::endl;
    sdrplay_api_LockDeviceApi();
    auto err = sdrplay_api_GetDevices(devices, &numDevs, SDRPLAY_MAX_DEVICES);

    std::cout << "GetDevices result: " << sdrplay_api_GetErrorString(err) << std::endl;
    std::cout << "Found " << numDevs << " devices" << std::endl;

    if (err == sdrplay_api_Success) {
        for (unsigned int i = 0; i < numDevs; i++) {
            DeviceInfo info;
            info.serialNumber = devices[i].SerNo;
            info.hwVer = devices[i].hwVer;
            info.tuner = static_cast<TunerSelect>(devices[i].tuner);
            info.valid = devices[i].valid;
            info.dev = devices[i].dev;
            
            std::cout << "Device " << i+1 << ": " << info.serialNumber 
                      << " (hwVer=" << static_cast<int>(info.hwVer) << ")" << std::endl;
            result.push_back(info);
        }
    } else {
        impl->lastError = sdrplay_api_GetErrorString(err);
        std::cerr << "Failed to get devices: " << impl->lastError << std::endl;
    }

    sdrplay_api_UnlockDeviceApi();
    return result;
}

bool DeviceControl::selectDevice(const DeviceInfo& deviceInfo) {
    if (!deviceInfo.valid) {
        throw DeviceException(ErrorCode::DEVICE_NOT_FOUND, 
                             "Device is not valid: " + deviceInfo.serialNumber);
    }

    sdrplay_api_DeviceT device;
    device.hwVer = deviceInfo.hwVer;
    device.tuner = static_cast<sdrplay_api_TunerSelectT>(deviceInfo.tuner);
    device.valid = deviceInfo.valid;
    device.dev = deviceInfo.dev;
    std::strncpy(device.SerNo, deviceInfo.serialNumber.c_str(), SDRPLAY_MAX_SER_NO_LEN - 1);
    device.SerNo[SDRPLAY_MAX_SER_NO_LEN - 1] = '\0'; // Ensure null termination

    auto err = sdrplay_api_SelectDevice(&device);
    if (err != sdrplay_api_Success) {
        std::string apiError = sdrplay_api_GetErrorString(err);
        impl->lastError = apiError;
        throw ApiException("Failed to select device: " + apiError);
    }

    // Store device handle
    if (impl->currentDevice) {
        delete impl->currentDevice;
    }
    impl->currentDevice = new sdrplay_api_DeviceT(device);

    // Get device parameters
    err = sdrplay_api_GetDeviceParams(device.dev, &impl->deviceParams);
    if (err != sdrplay_api_Success) {
        std::string apiError = sdrplay_api_GetErrorString(err);
        impl->lastError = apiError;
        throw ApiException("Failed to get device parameters: " + apiError);
    }

    return true;
}

bool DeviceControl::releaseDevice() {
    if (!impl->currentDevice) {
        throw DeviceException(ErrorCode::DEVICE_NOT_SELECTED, "No device selected to release");
    }
    
    auto err = sdrplay_api_ReleaseDevice(impl->currentDevice);
    if (err != sdrplay_api_Success) {
        std::string apiError = sdrplay_api_GetErrorString(err);
        impl->lastError = apiError;
        throw ApiException("Failed to release device: " + apiError);
    }
    
    delete impl->currentDevice;
    impl->currentDevice = nullptr;
    impl->deviceParams = nullptr;
    return true;
}

sdrplay_api_DeviceT* DeviceControl::getCurrentDevice() const {
    return impl->currentDevice;
}

sdrplay_api_DeviceParamsT* DeviceControl::getDeviceParams() const {
    return impl->deviceParams;
}

std::string DeviceControl::getLastError() const {
    return impl->lastError;
}

} // namespace sdrplay
