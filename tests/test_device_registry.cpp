#define SDRPLAY_TESTING
#include "device_registry.h"
#include "device_types.h"
#include "sdrplay_exception.h"
#include <cassert>
#include <iostream>

using namespace sdrplay;

class MockDeviceControl : public DeviceControl {
public:
    double freq = 100.0e6;
    double rate = 2.0e6;

    void setFrequency(double f) override { freq = f; }
    double getFrequency() const override { return freq; }
    void setSampleRate(double r) override { rate = r; }
    double getSampleRate() const override { return rate; }

    bool open() override { return true; }
    void close() override {}
    float getApiVersion() const override { return 3.15f; }
    std::vector<DeviceInfo> getAvailableDevices() override { return {}; }
    bool selectDevice(const DeviceInfo&) override { return true; }
    bool releaseDevice() override { return true; }
    sdrplay_api_DeviceT* getCurrentDevice() const override { return nullptr; }
    sdrplay_api_DeviceParamsT* getDeviceParams() const override { return nullptr; }
    std::string getLastError() const override { return ""; }
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
    } catch (const UnsupportedDeviceException& e) {
        std::cout << "Invalid device error caught correctly: " << e.getFullMessage() << std::endl;
        assert(e.getErrorCode() == ErrorCode::UNSUPPORTED_DEVICE);
    }
}

void testDeviceProperties() {
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
