// sdrplay_wrapper.cpp
#include "sdrplay_wrapper.h"
#include <stdexcept>
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cstring>

namespace sdrplay {

void StreamACallback(short* xi, short* xq, sdrplay_api_StreamCbParamsT* params,
                    unsigned int numSamples, unsigned int reset, void* cbContext) {
    std::cout << "StreamACallback entered with " << numSamples << " samples" << std::endl;
    auto* device = static_cast<Device*>(cbContext);
    if (!device) {
        std::cerr << "StreamACallback: null device pointer!" << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(device->pimpl->callbackMutex);

    if (device->pimpl->pythonStreamHandler) {
        std::cout << "Calling Python stream handler" << std::endl;
        device->pimpl->pythonStreamHandler->handleStreamData(xi, xq, numSamples);
        std::cout << "Python stream handler completed" << std::endl;
    }
    else {
        std::cout << "No Python stream handler registered" << std::endl;
    }
}

void EventCallback(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner,
                  sdrplay_api_EventParamsT* params, void* cbContext) {
    std::cout << "EventCallback entered with event " << eventId << std::endl;
    auto* device = static_cast<Device*>(cbContext);
    if (!device) {
        std::cerr << "EventCallback: null device pointer!" << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(device->pimpl->callbackMutex);

    switch (eventId) {
        case sdrplay_api_GainChange:
            if (device->pimpl->pythonGainHandler) {
                std::cout << "Calling Python gain handler" << std::endl;
                device->pimpl->pythonGainHandler->handleGainChange(
                    params->gainParams.gRdB,
                    params->gainParams.lnaGRdB,
                    params->gainParams.currGain
                );
                std::cout << "Python gain handler completed" << std::endl;
            }
            break;

        case sdrplay_api_PowerOverloadChange:
            if (device->pimpl->pythonPowerHandler) {
                std::cout << "Calling Python power handler" << std::endl;
                device->pimpl->pythonPowerHandler->handlePowerOverload(
                    params->powerOverloadParams.powerOverloadChangeType ==
                    sdrplay_api_Overload_Detected
                );
                std::cout << "Python power handler completed" << std::endl;
            }
            break;

        default:
            std::cout << "Unhandled event type: " << eventId << std::endl;
            break;
    }
}

// Device implementation
Device::Device() : pimpl(new Impl()) {}

Device::~Device() {
    close();
}

bool Device::open() {
    sdrplay_api_ErrT err = sdrplay_api_Open();
    if (err != sdrplay_api_Success) {
        pimpl->lastError = sdrplay_api_GetErrorString(err);
        return false;
    }

    // Wait for API to initialize
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Verify we can get API version after initialization
    float version;
    err = sdrplay_api_ApiVersion(&version);
    if (err != sdrplay_api_Success || version != SDRPLAY_API_VERSION) {
        pimpl->lastError = "API initialization failed or version mismatch";
        sdrplay_api_Close();
        return false;
    }

    return true;
}

void Device::close() {
    if (pimpl->device) {
        stopStreaming();
        sdrplay_api_ReleaseDevice(pimpl->device);
        pimpl->device = nullptr;
    }
    //sdrplay_api_Close();
}

bool Device::releaseDevice() {
    if (!pimpl->device) return false;

    sdrplay_api_ErrT err = sdrplay_api_ReleaseDevice(pimpl->device);
    if (err != sdrplay_api_Success) {
        pimpl->lastError = sdrplay_api_GetErrorString(err);
        return false;
    }
    pimpl->device = nullptr;
    return true;
}

float Device::getApiVersion() const {
    float version;
    sdrplay_api_ErrT err = sdrplay_api_ApiVersion(&version);
    if (err != sdrplay_api_Success) {
        pimpl->lastError = sdrplay_api_GetErrorString(err);
        // Return compile-time version if we can't get runtime version
        return SDRPLAY_API_VERSION;
    }
    return version;
}

DeviceParams* Device::getDeviceParams() {
    if (!pimpl->device || !pimpl->deviceParams) {
        std::cerr << "Device not properly initialized for parameter access" << std::endl;
        return nullptr;
    }
    return new DeviceParams(this);
}

RxChannelParams* Device::getRxChannelParams(bool isTunerB) {
    if (!pimpl->device) return nullptr;
    return new RxChannelParams(this, isTunerB);
}

std::vector<DeviceInfo> Device::getAvailableDevices() {
    std::vector<DeviceInfo> result;
    sdrplay_api_DeviceT devices[SDRPLAY_MAX_DEVICES];
    unsigned int numDevs;

    // Temporary storage for device data while under lock
    struct DeviceData {
        std::string serialNo;
        uint8_t hwVer;
        unsigned char tuner;
    };
    std::vector<DeviceData> deviceData;

    // Get devices under lock
    sdrplay_api_LockDeviceApi();
    sdrplay_api_ErrT err = sdrplay_api_GetDevices(devices, &numDevs, SDRPLAY_MAX_DEVICES);

    // Copy required data while still under lock
    if (err == sdrplay_api_Success) {
        deviceData.reserve(numDevs);
        for (unsigned int i = 0; i < numDevs; i++) {
            DeviceData data;
            data.serialNo = devices[i].SerNo;
            data.hwVer = devices[i].hwVer;
            data.tuner = devices[i].tuner;
            deviceData.push_back(data);
        }
    }
    sdrplay_api_UnlockDeviceApi();

    // Process the copied data outside the lock
    if (err == sdrplay_api_Success) {
        result.reserve(deviceData.size());
        for (const auto& data : deviceData) {
            DeviceInfo info;
            info.serialNumber = data.serialNo;
            info.hwVersion = data.hwVer;
            info.isTunerA = (data.tuner & sdrplay_api_Tuner_A) != 0;
            info.isTunerB = (data.tuner & sdrplay_api_Tuner_B) != 0;
            info.isRSPDuo = (data.hwVer == SDRPLAY_RSPduo_ID);
            result.push_back(info);
        }
    }

    // Debug output
    std::cout << "sdrplay_api_GetDevices result: " << sdrplay_api_GetErrorString(err) << std::endl;
    std::cout << "Number of devices found: " << (err == sdrplay_api_Success ? numDevs : 0) << std::endl;

    return result;
}

bool Device::selectDevice(const DeviceInfo& deviceInfo) {
    sdrplay_api_DeviceT devices[SDRPLAY_MAX_DEVICES];
    unsigned int numDevs;

    sdrplay_api_LockDeviceApi();
    sdrplay_api_ErrT err = sdrplay_api_GetDevices(devices, &numDevs, SDRPLAY_MAX_DEVICES);
    if (err != sdrplay_api_Success) {
        std::cerr << "GetDevices failed: " << sdrplay_api_GetErrorString(err) << std::endl;
        sdrplay_api_UnlockDeviceApi();
        return false;
    }

    // Find matching device
    bool found = false;
    for (unsigned int i = 0; i < numDevs; i++) {
        if (devices[i].SerNo == deviceInfo.serialNumber) {
            err = sdrplay_api_SelectDevice(&devices[i]);
            if (err == sdrplay_api_Success) {
                // Store a copy of the device
                pimpl->deviceStorage = devices[i];
                pimpl->device = &pimpl->deviceStorage;

                err = sdrplay_api_GetDeviceParams(pimpl->device->dev, &pimpl->deviceParams);
                if (err != sdrplay_api_Success) {
                    std::cerr << "GetDeviceParams failed: " << sdrplay_api_GetErrorString(err) << std::endl;
                    pimpl->device = nullptr;
                    break;
                }

                // Initialize the device with null callbacks for parameter updates
                sdrplay_api_CallbackFnsT callbacks = {};
                callbacks.StreamACbFn = nullptr;
                callbacks.StreamBCbFn = nullptr;
                callbacks.EventCbFn = nullptr;

                err = sdrplay_api_Init(pimpl->device->dev, &callbacks, nullptr);
                if (err != sdrplay_api_Success) {
                    std::cerr << "Device Init failed: " << sdrplay_api_GetErrorString(err) << std::endl;
                    pimpl->device = nullptr;
                } else {
                    found = true;
                    std::cout << "Device selected, parameters retrieved, and initialized successfully" << std::endl;
                }
            } else {
                std::cerr << "SelectDevice failed: " << sdrplay_api_GetErrorString(err) << std::endl;
            }
            break;
        }
    }

    sdrplay_api_UnlockDeviceApi();
    return found;
}


bool Device::startStreaming(StreamCallback streamCb, GainCallback gainCb, PowerOverloadCallback powerCb) {
    if (!pimpl->device) return false;

    pimpl->streamCallback = streamCb;
    pimpl->gainCallback = gainCb;
    pimpl->powerCallback = powerCb;

    sdrplay_api_CallbackFnsT callbacks;
    callbacks.StreamACbFn = &sdrplay::StreamACallback;
    callbacks.StreamBCbFn = nullptr;
    callbacks.EventCbFn = &sdrplay::EventCallback;

    sdrplay_api_ErrT err = sdrplay_api_Init(pimpl->device->dev, &callbacks, this);
    return (err == sdrplay_api_Success);
}

bool Device::startStreamingWithHandlers(StreamCallbackHandler* streamHandler,
                                      GainCallbackHandler* gainHandler,
                                      PowerOverloadCallbackHandler* powerHandler) {
    std::cout << "startStreamingWithHandlers called" << std::endl;
    if (!pimpl->device) {
        std::cerr << "No device selected!" << std::endl;
        return false;
    }

    // Need to uninit first since we initialized in selectDevice
    sdrplay_api_Uninit(pimpl->device->dev);

    // Store Python handlers
    pimpl->pythonStreamHandler = streamHandler;
    pimpl->pythonGainHandler = gainHandler;
    pimpl->pythonPowerHandler = powerHandler;

    sdrplay_api_CallbackFnsT callbacks;
    callbacks.StreamACbFn = &sdrplay::StreamACallback;
    callbacks.StreamBCbFn = nullptr;
    callbacks.EventCbFn = &sdrplay::EventCallback;

    std::cout << "Calling sdrplay_api_Init with streaming callbacks" << std::endl;
    sdrplay_api_ErrT err = sdrplay_api_Init(pimpl->device->dev, &callbacks, this);
    if (err != sdrplay_api_Success) {
        std::cerr << "sdrplay_api_Init failed: " << sdrplay_api_GetErrorString(err) << std::endl;
        pimpl->pythonStreamHandler = nullptr;
        pimpl->pythonGainHandler = nullptr;
        pimpl->pythonPowerHandler = nullptr;
        return false;
    }

    std::cout << "Streaming initialized successfully" << std::endl;
    return true;
}


// Make sure to clear Python handlers in stopStreaming
bool Device::stopStreaming() {
    if (!pimpl->device) return false;

    sdrplay_api_ErrT err = sdrplay_api_Uninit(pimpl->device->dev);

    // Clear Python handlers
    pimpl->pythonStreamHandler = nullptr;
    pimpl->pythonGainHandler = nullptr;
    pimpl->pythonPowerHandler = nullptr;

    return (err == sdrplay_api_Success);
}

std::string Device::getLastErrorMessage() const {
    return pimpl->lastError;
}

// DeviceParams implementation
DeviceParams::DeviceParams(Device* device) : pimpl(new Impl(device, device->pimpl->deviceParams)) {
    if (!device || !device->pimpl->deviceParams) {
        std::cerr << "Invalid device or parameters in DeviceParams constructor" << std::endl;
        throw std::runtime_error("Invalid device parameters");
    }
}

void DeviceParams::setSampleRate(double sampleRateHz) {
    if (pimpl->params && pimpl->params->devParams) {
        pimpl->params->devParams->fsFreq.fsHz = sampleRateHz;
    }
}

void DeviceParams::setPpm(double ppm) {
    if (pimpl->params && pimpl->params->devParams) {
        pimpl->params->devParams->ppm = ppm;
    }
}

bool DeviceParams::update() {
    std::cout << "DeviceParams::update() called" << std::endl;

    if (!pimpl) {
        std::cerr << "DeviceParams::update() - pimpl is null" << std::endl;
        return false;
    }

    if (!pimpl->device) {
        std::cerr << "DeviceParams::update() - device is null" << std::endl;
        return false;
    }

    if (!pimpl->device->pimpl->device) {
        std::cerr << "DeviceParams::update() - device->pimpl->device is null" << std::endl;
        return false;
    }

    std::cout << "DeviceParams::update() - Calling sdrplay_api_Update" << std::endl;
    sdrplay_api_ReasonForUpdateT reason =
        static_cast<sdrplay_api_ReasonForUpdateT>(
            sdrplay_api_Update_Dev_Fs |
            sdrplay_api_Update_Dev_Ppm
        );

    sdrplay_api_ErrT err = sdrplay_api_Update(
        pimpl->device->pimpl->device->dev,
        pimpl->device->pimpl->device->tuner,
        reason,
        sdrplay_api_Update_Ext1_None
    );

    std::cout << "DeviceParams::update() - sdrplay_api_Update returned: "
              << sdrplay_api_GetErrorString(err) << std::endl;

    return (err == sdrplay_api_Success);
}

// RxChannelParams implementation
RxChannelParams::RxChannelParams(Device* device, bool isTunerB)
    : pimpl(new Impl(device, nullptr, isTunerB)) {
    if (device->pimpl->deviceParams) {
        pimpl->params = isTunerB ? device->pimpl->deviceParams->rxChannelB
                                : device->pimpl->deviceParams->rxChannelA;
    }
}

void RxChannelParams::setRfFrequency(double frequencyHz) {
    if (pimpl->params) {
        pimpl->params->tunerParams.rfFreq.rfHz = frequencyHz;
    }
}

void RxChannelParams::setBandwidth(int bandwidthKHz) {
    if (pimpl->params) {
        switch (bandwidthKHz) {
            case 200: pimpl->params->tunerParams.bwType = sdrplay_api_BW_0_200; break;
            case 300: pimpl->params->tunerParams.bwType = sdrplay_api_BW_0_300; break;
            case 600: pimpl->params->tunerParams.bwType = sdrplay_api_BW_0_600; break;
            case 1536: pimpl->params->tunerParams.bwType = sdrplay_api_BW_1_536; break;
            case 5000: pimpl->params->tunerParams.bwType = sdrplay_api_BW_5_000; break;
            case 6000: pimpl->params->tunerParams.bwType = sdrplay_api_BW_6_000; break;
            case 7000: pimpl->params->tunerParams.bwType = sdrplay_api_BW_7_000; break;
            case 8000: pimpl->params->tunerParams.bwType = sdrplay_api_BW_8_000; break;
            default: pimpl->params->tunerParams.bwType = sdrplay_api_BW_0_200; break;
        }
    }
}

void RxChannelParams::setIFType(int ifkHz) {
    if (pimpl->params) {
        switch (ifkHz) {
            case 0: pimpl->params->tunerParams.ifType = sdrplay_api_IF_Zero; break;
            case 450: pimpl->params->tunerParams.ifType = sdrplay_api_IF_0_450; break;
            case 1620: pimpl->params->tunerParams.ifType = sdrplay_api_IF_1_620; break;
            case 2048: pimpl->params->tunerParams.ifType = sdrplay_api_IF_2_048; break;
            default: pimpl->params->tunerParams.ifType = sdrplay_api_IF_Zero; break;
        }
    }
}

void RxChannelParams::setGain(int gainReduction, int lnaState) {
    if (pimpl->params) {
        pimpl->params->tunerParams.gain.gRdB = gainReduction;
        pimpl->params->tunerParams.gain.LNAstate = lnaState;
    }
}

void RxChannelParams::setAgcControl(bool enable, int setPoint) {
    if (pimpl->params) {
        pimpl->params->ctrlParams.agc.enable = enable ? sdrplay_api_AGC_CTRL_EN : sdrplay_api_AGC_DISABLE;
        pimpl->params->ctrlParams.agc.setPoint_dBfs = setPoint;
    }
}

bool RxChannelParams::update() {
    if (!pimpl->device->pimpl->device) return false;

    sdrplay_api_ReasonForUpdateT reason =
        static_cast<sdrplay_api_ReasonForUpdateT>(
            sdrplay_api_Update_Tuner_Frf |
            sdrplay_api_Update_Tuner_BwType |
            sdrplay_api_Update_Tuner_IfType |
            sdrplay_api_Update_Tuner_Gr |
            sdrplay_api_Update_Ctrl_Agc
        );

    sdrplay_api_ErrT err = sdrplay_api_Update(
        pimpl->device->pimpl->device->dev,
        pimpl->isTunerB ? sdrplay_api_Tuner_B : sdrplay_api_Tuner_A,
        reason,
        sdrplay_api_Update_Ext1_None
    );
    return (err == sdrplay_api_Success);
}

} // namespace sdrplay
