#include "device_control.h"
#include "sdrplay_exception.h"
#include <cstring>
#include <iostream>

namespace sdrplay {

struct DeviceControl::Impl {
    sdrplay_api_DeviceT* currentDevice{nullptr};
    sdrplay_api_DeviceParamsT* deviceParams{nullptr};
    std::string lastError;
    std::unique_ptr<CallbackWrapper> callbackWrapper;
    bool isStreaming{false};
    sdrplay_api_CallbackFnsT callbackFunctions;
};

DeviceControl::DeviceControl() : impl(std::make_unique<Impl>()) {
    impl->callbackWrapper = std::make_unique<CallbackWrapper>();
}

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
        // Stop streaming if active
        if (impl->isStreaming) {
            stopStreaming();
        }
        
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
    
    // Stop streaming if active
    if (impl->isStreaming) {
        stopStreaming();
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

bool DeviceControl::startStreaming(const StreamingParams& params) {
    if (!impl->currentDevice || !impl->deviceParams) {
        impl->lastError = "No device selected";
        return false;
    }
    
    if (impl->isStreaming) {
        // Already streaming
        return true;
    }
    
    // Set up streaming parameters
    if (!setupStreamingParameters(params)) {
        return false;
    }
    
    // Set up callback functions
    impl->callbackFunctions.StreamACbFn = impl->callbackWrapper->getStreamCallback();
    impl->callbackFunctions.StreamBCbFn = nullptr;  // Not using stream B
    impl->callbackFunctions.EventCbFn = impl->callbackWrapper->getEventCallback();
    
    // Initialize streaming
    sdrplay_api_ErrT err = sdrplay_api_Init(
        impl->currentDevice->dev, 
        &impl->callbackFunctions, 
        impl->callbackWrapper->getContext()
    );
    
    if (err != sdrplay_api_Success) {
        impl->lastError = sdrplay_api_GetErrorString(err);
        std::cerr << "Failed to start streaming: " << impl->lastError << std::endl;
        return false;
    }
    
    impl->isStreaming = true;
    return true;
}

bool DeviceControl::stopStreaming() {
    if (!impl->currentDevice || !impl->isStreaming) {
        return true;  // Not streaming, so nothing to stop
    }
    
    // Uninitialize API to stop streaming
    sdrplay_api_ErrT err = sdrplay_api_Uninit(impl->currentDevice->dev);
    if (err != sdrplay_api_Success) {
        impl->lastError = sdrplay_api_GetErrorString(err);
        std::cerr << "Failed to stop streaming: " << impl->lastError << std::endl;
        return false;
    }
    
    impl->isStreaming = false;
    return true;
}

bool DeviceControl::isStreaming() const {
    return impl->isStreaming;
}

void DeviceControl::setSampleCallback(CallbackWrapper::SampleCallback callback) {
    if (impl->callbackWrapper) {
        impl->callbackWrapper->setSampleCallback(callback);
    }
}

void DeviceControl::setEventCallback(CallbackWrapper::EventCallback callback) {
    if (impl->callbackWrapper) {
        impl->callbackWrapper->setEventCallback(callback);
    }
}

bool DeviceControl::waitForSamples(size_t count, unsigned int timeoutMs) {
    if (!impl->callbackWrapper || !impl->isStreaming) {
        return false;
    }
    return impl->callbackWrapper->waitForSamples(count, timeoutMs);
}

size_t DeviceControl::readSamples(std::complex<short>* dest, size_t maxCount) {
    if (!impl->callbackWrapper || !impl->isStreaming) {
        return 0;
    }
    return impl->callbackWrapper->readSamples(dest, maxCount);
}

size_t DeviceControl::samplesAvailable() const {
    if (!impl->callbackWrapper || !impl->isStreaming) {
        return 0;
    }
    return impl->callbackWrapper->samplesAvailable();
}

bool DeviceControl::hasBufferOverflow() const {
    if (!impl->callbackWrapper) {
        return false;
    }
    return impl->callbackWrapper->hasOverflow();
}

void DeviceControl::resetBuffer() {
    if (impl->callbackWrapper) {
        impl->callbackWrapper->resetBuffer();
    }
}

CallbackWrapper* DeviceControl::getCallbackWrapper() {
    return impl->callbackWrapper.get();
}

bool DeviceControl::setupStreamingParameters(const StreamingParams& params) {
    if (!impl->deviceParams) {
        impl->lastError = "No device parameters available";
        return false;
    }
    
    // Configure IQ correction and DC offset
    impl->deviceParams->rxChannelA->ctrlParams.dcOffset.DCenable = 
        params.enableDCCorrection ? 1 : 0;
    impl->deviceParams->rxChannelA->ctrlParams.dcOffset.IQenable = 
        params.enableIQCorrection ? 1 : 0;
    
    // Configure decimation
    impl->deviceParams->rxChannelA->ctrlParams.decimation.enable = 
        params.decimate ? 1 : 0;
    impl->deviceParams->rxChannelA->ctrlParams.decimation.decimationFactor = 
        params.decimationFactor;
    impl->deviceParams->rxChannelA->ctrlParams.decimation.wideBandSignal = 
        params.wideBandSignal ? 1 : 0;
    
    // Update device with these parameters
    sdrplay_api_ErrT err = sdrplay_api_Update(
        impl->currentDevice->dev, 
        impl->currentDevice->tuner, 
        sdrplay_api_Update_Ctrl_DCoffsetIQimbalance, 
        sdrplay_api_Update_Ext1_None
    );
    
    if (err != sdrplay_api_Success) {
        impl->lastError = sdrplay_api_GetErrorString(err);
        std::cerr << "Failed to update DC offset/IQ parameters: " << impl->lastError << std::endl;
        return false;
    }
    
    err = sdrplay_api_Update(
        impl->currentDevice->dev, 
        impl->currentDevice->tuner, 
        sdrplay_api_Update_Ctrl_Decimation, 
        sdrplay_api_Update_Ext1_None
    );
    
    if (err != sdrplay_api_Success) {
        impl->lastError = sdrplay_api_GetErrorString(err);
        std::cerr << "Failed to update decimation parameters: " << impl->lastError << std::endl;
        return false;
    }
    
    return true;
}

} // namespace sdrplay