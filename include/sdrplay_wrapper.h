// sdrplay_wrapper.h
#ifndef SDRPLAY_WRAPPER_H
#define SDRPLAY_WRAPPER_H

#include <stdint.h>
#include <functional>
#include <memory>
#include <vector>
#include <string>

namespace sdrplay {

// Forward declarations
class DeviceParams;
class RxChannelParams;

// Wrapper for callback functions to make them more idiomatic
using StreamCallback = std::function<void(const int16_t* xi, const int16_t* xq, size_t numSamples)>;
using GainCallback = std::function<void(int gRdB, int lnaGRdB, double currGain)>;
using PowerOverloadCallback = std::function<void(bool isOverloaded)>;

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
    struct DeviceInfo {
        std::string serialNumber;
        uint8_t hwVersion;
        bool isTunerA;
        bool isTunerB;
        bool isRSPDuo;
    };

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
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

// Parameter classes to control device settings
class DeviceParams {
public:
    void setSampleRate(double sampleRateHz);
    void setPpm(double ppm);
    bool update();

private:
    friend class Device;
    DeviceParams(Device* device);
    struct Impl;
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
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace sdrplay

#endif // SDRPLAY_WRAPPER_H
