// sdrplay_wrapper.h
#ifndef SDRPLAY_WRAPPER_H
#define SDRPLAY_WRAPPER_H

#include <stdint.h>
#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <mutex>
#include "sdrplay_api.h"

namespace sdrplay {

// Forward declarations
class Device;
class DeviceParams;
class RxChannelParams;

// Move DeviceInfo out of Device class for SWIG
struct DeviceInfo {
    std::string serialNumber;
    uint8_t hwVersion;
    bool isTunerA;
    bool isTunerB;
    bool isRSPDuo;
};

// Wrapper for callback functions to make them more idiomatic
using StreamCallback = std::function<void(const int16_t* xi, const int16_t* xq, size_t numSamples)>;
using GainCallback = std::function<void(int gRdB, int lnaGRdB, double currGain)>;
using PowerOverloadCallback = std::function<void(bool isOverloaded)>;

// Forward declare callback functions
namespace detail {
    void StreamACallback(short* xi, short* xq, sdrplay_api_StreamCbParamsT* params,
                        unsigned int numSamples, unsigned int reset, void* cbContext);
    void EventCallback(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner,
                      sdrplay_api_EventParamsT* params, void* cbContext);
}

// Parameter classes to control device settings
class DeviceParams {
public:
    void setSampleRate(double sampleRateHz);
    void setPpm(double ppm);
    bool update();

private:
    friend class Device;
    explicit DeviceParams(Device* device);

    struct Impl {
        Device* device;
        sdrplay_api_DeviceParamsT* params;

        Impl(Device* d, sdrplay_api_DeviceParamsT* p) : device(d), params(p) {}
    };
    std::unique_ptr<Impl> pimpl;
};

class RxChannelParams {
public:
    void setRfFrequency(double frequencyHz);
    void setBandwidth(int bandwidthKHz);
    void setIFType(int ifkHz);
    void setGain(int gainReduction, int lnaState);
    void setAgcControl(bool enable, int setPoint = -60);
    bool update();

private:
    friend class Device;
    RxChannelParams(Device* device, bool isTunerB);

    struct Impl {
        Device* device;
        sdrplay_api_RxChannelParamsT* params;
        bool isTunerB;

        Impl(Device* d, sdrplay_api_RxChannelParamsT* p, bool b)
            : device(d), params(p), isTunerB(b) {}
    };
    std::unique_ptr<Impl> pimpl;
};

// Main device wrapper class
class Device {
public:
    // Constructor/destructor
    Device();
    ~Device();

    // Prevent copying
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    // Core API functions
    bool open();
    void close();
    float getApiVersion() const;

    // Device selection and configuration
    std::vector<DeviceInfo> getAvailableDevices();
    bool selectDevice(const DeviceInfo& device);
    bool releaseDevice();

    // Parameter access and control
    DeviceParams* getDeviceParams();
    RxChannelParams* getRxChannelParams(bool isTunerB = false);

    // Streaming control
    bool startStreaming(StreamCallback streamCb,
                       GainCallback gainCb = nullptr,
                       PowerOverloadCallback powerCb = nullptr);
    bool stopStreaming();

    // Error handling
    std::string getLastErrorMessage() const;

private:
    struct Impl {
        sdrplay_api_DeviceT* device;
        sdrplay_api_DeviceParamsT* deviceParams;
        StreamCallback streamCallback;
        GainCallback gainCallback;
        PowerOverloadCallback powerCallback;
        std::mutex callbackMutex;
        std::string lastError;

        Impl() : device(nullptr), deviceParams(nullptr) {}
    };
    std::unique_ptr<Impl> pimpl;

    // Grant access to callbacks and related classes
    friend void detail::StreamACallback(short* xi, short* xq, sdrplay_api_StreamCbParamsT* params,
                                      unsigned int numSamples, unsigned int reset, void* cbContext);
    friend void detail::EventCallback(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner,
                                    sdrplay_api_EventParamsT* params, void* cbContext);
    friend class DeviceParams;
    friend class RxChannelParams;
};

} // namespace sdrplay

#endif // SDRPLAY_WRAPPER_H
