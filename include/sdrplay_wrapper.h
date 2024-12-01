// sdrplay_wrapper.h
#ifndef SDRPLAY_WRAPPER_H
#define SDRPLAY_WRAPPER_H

#include <cstdint>
#include <fstream>
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
class StreamCallbackHandler;
class GainCallbackHandler;
class PowerOverloadCallbackHandler;

// Move DeviceInfo out of Device class for SWIG
struct DeviceInfo {
    std::string serialNumber;
    int hwVersion;  // Changed from uint8_t to int for better SWIG compatibility
    bool isTunerA;
    bool isTunerB;
    bool isRSPDuo;

    // Add constructor for proper initialization
    DeviceInfo() : hwVersion(0), isTunerA(false), isTunerB(false), isRSPDuo(false) {}
};

struct ApiTiming {
    // Time to wait after opening API
    std::chrono::milliseconds openDelay;

    // Time to wait after device selection
    std::chrono::milliseconds selectDelay;

    // Time to wait after parameter updates
    std::chrono::milliseconds updateDelay;

    // Time to wait after device initialization
    std::chrono::milliseconds initDelay;

    // Default timings for different platforms
    static ApiTiming getDefaultTiming() {
        ApiTiming timing;

        // Check if we're on a Raspberry Pi
        std::ifstream cpuinfo("/proc/cpuinfo");
        std::string line;
        bool isRaspberryPi = false;

        while (std::getline(cpuinfo, line)) {
            if (line.find("Raspberry Pi") != std::string::npos) {
                isRaspberryPi = true;
                break;
            }
        }

        if (isRaspberryPi) {
            // Conservative timings for Raspberry Pi
            timing.openDelay = std::chrono::milliseconds(2000);    // 2 seconds
            timing.selectDelay = std::chrono::milliseconds(1000);  // 1 second
            timing.updateDelay = std::chrono::milliseconds(500);   // 500ms
            timing.initDelay = std::chrono::milliseconds(1500);    // 1.5 seconds
        } else {
            // Faster timings for desktop systems
            timing.openDelay = std::chrono::milliseconds(1000);    // 1 second
            timing.selectDelay = std::chrono::milliseconds(500);   // 500ms
            timing.updateDelay = std::chrono::milliseconds(200);   // 200ms
            timing.initDelay = std::chrono::milliseconds(750);     // 750ms
        }

        return timing;
    }
};

// Wrapper for callback functions to make them more idiomatic
using StreamCallback = std::function<void(const int16_t* xi, const int16_t* xq, size_t numSamples)>;
using GainCallback = std::function<void(int gRdB, int lnaGRdB, double currGain)>;
using PowerOverloadCallback = std::function<void(bool isOverloaded)>;

// Abstract base classes for callbacks
class StreamCallbackHandler {
public:
    virtual ~StreamCallbackHandler() = default;
    virtual void handleStreamData(const int16_t* xi, const int16_t* xq, size_t numSamples) = 0;
};

class GainCallbackHandler {
public:
    virtual ~GainCallbackHandler() = default;
    virtual void handleGainChange(int gRdB, int lnaGRdB, double currGain) = 0;
};

class PowerOverloadCallbackHandler {
public:
    virtual ~PowerOverloadCallbackHandler() = default;
    virtual void handlePowerOverload(bool isOverloaded) = 0;
};

// Forward declare callback functions
void StreamACallback(short* xi, short* xq, sdrplay_api_StreamCbParamsT* params,
                    unsigned int numSamples, unsigned int reset, void* cbContext);
void EventCallback(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner,
                    sdrplay_api_EventParamsT* params, void* cbContext);

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

    // Add timing configuration methods
    void setTiming(const ApiTiming& timing) { pimpl->timing = timing; }
    ApiTiming getTiming() const { return pimpl->timing; }

    // Streaming control
    bool startStreaming(StreamCallback streamCb,
                       GainCallback gainCb = nullptr,
                       PowerOverloadCallback powerCb = nullptr);

    bool startStreamingWithHandlers(StreamCallbackHandler* streamHandler = nullptr,
                                  GainCallbackHandler* gainHandler = nullptr,
                                  PowerOverloadCallbackHandler* powerHandler = nullptr);

    bool stopStreaming();

    // Error handling
    std::string getLastErrorMessage() const;

private:
    struct Impl {
        sdrplay_api_DeviceT deviceStorage;  // Store the actual device, not just a pointer
        sdrplay_api_DeviceT* device;        // Pointer to our stored device
        sdrplay_api_DeviceParamsT* deviceParams;
        ApiTiming timing;
        StreamCallback streamCallback;
        GainCallback gainCallback;
        PowerOverloadCallback powerCallback;
        StreamCallbackHandler* pythonStreamHandler;
        GainCallbackHandler* pythonGainHandler;
        PowerOverloadCallbackHandler* pythonPowerHandler;
        std::mutex callbackMutex;
        std::string lastError;

        Impl() : device(nullptr),
                deviceParams(nullptr),
                timing(ApiTiming::getDefaultTiming()),
                pythonStreamHandler(nullptr),
                pythonGainHandler(nullptr),
                pythonPowerHandler(nullptr) {}
    };
    std::unique_ptr<Impl> pimpl;

    // Grant access to callbacks and related classes
    friend void sdrplay::StreamACallback(short* xi, short* xq, sdrplay_api_StreamCbParamsT* params,
                                      unsigned int numSamples, unsigned int reset, void* cbContext);
    friend void sdrplay::EventCallback(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner,
                                    sdrplay_api_EventParamsT* params, void* cbContext);
    friend class DeviceParams;
    friend class RxChannelParams;
};

} // namespace sdrplay

#endif // SDRPLAY_WRAPPER_H
