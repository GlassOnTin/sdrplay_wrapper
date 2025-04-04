#define SDRPLAY_TESTING
#include <cassert>
#include <iostream>
#include <memory>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <functional>

// Minimal mocks for testing the device registry

// Mock API types
typedef void* HANDLE;
struct sdrplay_api_DeviceT { };
struct sdrplay_api_DeviceParamsT { };

// MockDeviceInfo
struct DeviceInfo {
    std::string serialNumber;
    unsigned char hwVer;
    bool valid;
    void* dev;

    DeviceInfo() : hwVer(0), valid(false), dev(nullptr) {}
};

// Constants
constexpr unsigned char RSP1A_HWVER = 255;
constexpr unsigned char RSPDXR2_HWVER = 7;

// Forward declarations for mock callback handlers
class StreamCallbackHandler;
class GainCallbackHandler;
class PowerOverloadCallbackHandler;

// Mock DeviceControl
class DeviceControl {
public:
    DeviceControl() {}
    virtual ~DeviceControl() {}

    // Device management
    virtual bool open() { return true; }
    virtual void close() {}
    virtual float getApiVersion() const { return 3.15f; }
    virtual std::vector<DeviceInfo> getAvailableDevices() { return {}; }
    virtual bool selectDevice(const DeviceInfo&) { return true; }
    virtual bool releaseDevice() { return true; }

    // Device access
    virtual sdrplay_api_DeviceT* getCurrentDevice() const { return nullptr; }
    virtual sdrplay_api_DeviceParamsT* getDeviceParams() const { return nullptr; }
    virtual std::string getLastError() const { return ""; }

    // Common control methods
    virtual void setFrequency(double freq) = 0;
    virtual double getFrequency() const = 0;
    virtual void setSampleRate(double rate) = 0;
    virtual double getSampleRate() const = 0;

    // Device specific controls
    virtual void setGainReduction(int) = 0;
    virtual void setLNAState(int) = 0;
    virtual void setHDRMode(bool) = 0;
    virtual void setBiasTEnabled(bool) = 0;
    
    // Streaming control methods
    virtual bool initializeStreaming() { return true; }
    virtual bool startStreaming() { return true; }
    virtual bool stopStreaming() { return true; }
    virtual bool isStreaming() const { return false; }
    
    // Callback registration
    virtual void setStreamCallback(StreamCallbackHandler*) {}
    virtual void setGainCallback(GainCallbackHandler*) {}
    virtual void setPowerOverloadCallback(PowerOverloadCallbackHandler*) {}
};

// Mock device factory 
using DeviceControlFactory = std::function<std::unique_ptr<DeviceControl>()>;

// Mock device registry
class DeviceRegistry {
public:
    static std::map<unsigned char, DeviceControlFactory>& getFactoryMap() {
        static std::map<unsigned char, DeviceControlFactory> factories;
        return factories;
    }

    static void registerFactory(unsigned char hwVer, DeviceControlFactory factory) {
        getFactoryMap()[hwVer] = factory;
    }

    static std::unique_ptr<DeviceControl> createDeviceControl(unsigned char hwVer) {
        auto& factories = getFactoryMap();
        auto it = factories.find(hwVer);
        if (it == factories.end()) {
            throw std::runtime_error("Unsupported device hardware version");
        }
        return it->second();
    }

    static void clearFactories() {
        getFactoryMap().clear();
    }
};

// Specific mock implementation
class MockDeviceControl : public DeviceControl {
public:
    double freq = 100.0e6;
    double rate = 2.0e6;

    void setFrequency(double f) override { freq = f; }
    double getFrequency() const override { return freq; }
    void setSampleRate(double r) override { rate = r; }
    double getSampleRate() const override { return rate; }

    void setGainReduction(int) override {}
    void setLNAState(int) override {}
    void setHDRMode(bool) override {}
    void setBiasTEnabled(bool) override {}
};

void testDeviceCreation() {
    std::cout << "Testing device creation..." << std::endl;

    // Register mock factories
    DeviceRegistry::registerFactory(RSP1A_HWVER,
        []() { return std::make_unique<MockDeviceControl>(); });
    DeviceRegistry::registerFactory(RSPDXR2_HWVER,
        []() { return std::make_unique<MockDeviceControl>(); });

    // Test RSP1A
    auto rsp1a = DeviceRegistry::createDeviceControl(RSP1A_HWVER);
    assert(rsp1a != nullptr);
    auto* mock1 = dynamic_cast<MockDeviceControl*>(rsp1a.get());
    assert(mock1 != nullptr);
    std::cout << "RSP1A mock created and verified" << std::endl;

    // Test RSPdxR2
    auto rspdx = DeviceRegistry::createDeviceControl(RSPDXR2_HWVER);
    assert(rspdx != nullptr);
    auto* mock2 = dynamic_cast<MockDeviceControl*>(rspdx.get());
    assert(mock2 != nullptr);
    std::cout << "RSPdxR2 mock created and verified" << std::endl;

    // Test invalid device
    try {
        DeviceRegistry::createDeviceControl(123);
        assert(false);
    } catch (const std::exception& e) {
        std::cout << "Invalid device error caught correctly: " << e.what() << std::endl;
    }

    // Clear factories for other tests
    DeviceRegistry::clearFactories();
}

void testDeviceProperties() {
    // Create new factories for this test
    DeviceRegistry::registerFactory(RSP1A_HWVER,
        []() { return std::make_unique<MockDeviceControl>(); });

    auto device = DeviceRegistry::createDeviceControl(RSP1A_HWVER);
    assert(device->getFrequency() == 100.0e6);

    device->setFrequency(200.0e6);
    assert(device->getFrequency() == 200.0e6);

    device->setSampleRate(8.0e6);
    assert(device->getSampleRate() == 8.0e6);

    std::cout << "Device properties verified" << std::endl;
}

int main() {
    try {
        testDeviceCreation();
        testDeviceProperties();
        std::cout << "All tests passed" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}