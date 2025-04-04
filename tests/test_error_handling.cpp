#define SDRPLAY_TESTING
#include "device_registry.h"
#include "device_types.h"
#include "sdrplay_exception.h"
#include <cassert>
#include <iostream>
#include <functional>
#include <vector>
#include <string>

using namespace sdrplay;

class MockDeviceControl : public DeviceControl {
public:
    bool failOnSelectDevice = false;
    bool failOnReleaseDevice = false;
    bool failOnInit = false;
    bool failOnUninit = false;
    
    double freq = 100.0e6;
    double rate = 2.0e6;
    bool streaming = false;

    void setFrequency(double f) override { freq = f; }
    double getFrequency() const override { return freq; }
    void setSampleRate(double r) override { rate = r; }
    double getSampleRate() const override { return rate; }

    bool open() override { return true; }
    void close() override {}
    float getApiVersion() const override { return 3.15f; }
    std::vector<DeviceInfo> getAvailableDevices() override { return {}; }
    
    bool selectDevice(const DeviceInfo& deviceInfo) override { 
        if (!deviceInfo.valid) {
            throw DeviceException(ErrorCode::DEVICE_NOT_FOUND, 
                                 "Device is not valid: Mock device");
        }
        
        if (failOnSelectDevice) {
            throw ApiException("Mock API error: Failed to select device");
        }
        return true; 
    }
    
    bool releaseDevice() override { 
        if (!getCurrentDevice()) {
            throw DeviceException(ErrorCode::DEVICE_NOT_SELECTED, 
                                "No device selected to release");
        }
        
        if (failOnReleaseDevice) {
            throw ApiException("Mock API error: Failed to release device");
        }
        return true; 
    }
    
    sdrplay_api_DeviceT* getCurrentDevice() const override { return nullptr; }
    sdrplay_api_DeviceParamsT* getDeviceParams() const override { return nullptr; }
    std::string getLastError() const override { return ""; }
    void setGainReduction(int) override {}
    void setLNAState(int) override {}
    void setHDRMode(bool) override {}
    void setBiasTEnabled(bool) override {}
    
    // Streaming API mock implementations
    bool initializeStreaming() override { 
        if (getCurrentDevice() == nullptr) {
            throw DeviceException(ErrorCode::DEVICE_NOT_SELECTED, 
                                 "No device selected for streaming initialization");
        }
        return true; 
    }
    
    bool startStreaming() override { 
        if (streaming) {
            throw StreamingException(ErrorCode::STREAMING_ALREADY_ACTIVE, 
                                    "Streaming is already active");
        }
        
        if (failOnInit) {
            throw ApiException("Mock API error: Failed to initialize streaming");
        }
        
        streaming = true; 
        return true; 
    }
    
    bool stopStreaming() override { 
        if (!streaming) {
            return true; // Not an error, just a no-op
        }
        
        if (failOnUninit) {
            throw ApiException("Mock API error: Failed to uninitialize streaming");
        }
        
        streaming = false; 
        return true; 
    }
    
    bool isStreaming() const override { return streaming; }
    
    // Callback registration mocks
    void setStreamCallback(StreamCallbackHandler*) override {}
    void setGainCallback(GainCallbackHandler*) override {}
    void setPowerOverloadCallback(PowerOverloadCallbackHandler*) override {}
};

// Test helper to run a function and catch expected exceptions
template<typename ExceptionType>
bool expectException(std::function<void()> func, ErrorCode expectedCode, const std::string& contains = "") {
    try {
        func();
        std::cerr << "Expected exception, but none was thrown" << std::endl;
        return false;
    }
    catch (const ExceptionType& e) {
        if (e.getErrorCode() != expectedCode) {
            std::cerr << "Expected error code " << static_cast<int>(expectedCode) 
                      << " but got " << static_cast<int>(e.getErrorCode()) << std::endl;
            return false;
        }
        
        if (!contains.empty() && std::string(e.what()).find(contains) == std::string::npos) {
            std::cerr << "Exception message does not contain '" << contains 
                      << "': " << e.what() << std::endl;
            return false;
        }
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Wrong exception type caught: " << e.what() << std::endl;
        return false;
    }
}

// Test unsupported device exception
void testUnsupportedDevice() {
    std::cout << "Testing unsupported device exception..." << std::endl;
    
    DeviceRegistry::clearFactories();
    
    // Test invalid device
    bool result = expectException<UnsupportedDeviceException>(
        []() { DeviceRegistry::createDeviceControl(123); },
        ErrorCode::UNSUPPORTED_DEVICE,
        "123"
    );
    
    assert(result);
    std::cout << "Unsupported device exception test passed" << std::endl;
}

// Test device exceptions
void testDeviceExceptions() {
    std::cout << "Testing device exceptions..." << std::endl;
    
    // Register mock factory
    DeviceRegistry::registerFactory(RSP1A_HWVER,
        []() { return std::make_unique<MockDeviceControl>(); });
    
    // Create a mock device
    auto device = DeviceRegistry::createDeviceControl(RSP1A_HWVER);
    auto* mockDevice = dynamic_cast<MockDeviceControl*>(device.get());
    assert(mockDevice != nullptr);
    
    // Test device not selected exception
    DeviceInfo invalidDevice;
    invalidDevice.valid = false;
    
    bool result = expectException<DeviceException>(
        [&device, &invalidDevice]() { device->selectDevice(invalidDevice); },
        ErrorCode::DEVICE_NOT_FOUND
    );
    assert(result);
    
    // Test API exception during selectDevice
    mockDevice->failOnSelectDevice = true;
    DeviceInfo validDevice;
    validDevice.valid = true;
    
    result = expectException<ApiException>(
        [&device, &validDevice]() { device->selectDevice(validDevice); },
        ErrorCode::API_ERROR,
        "Failed to select device"
    );
    assert(result);
    
    // Test release device without selecting one first
    result = expectException<DeviceException>(
        [&device]() { device->releaseDevice(); },
        ErrorCode::DEVICE_NOT_SELECTED
    );
    assert(result);
    
    std::cout << "Device exceptions test passed" << std::endl;
}

// Test streaming exceptions
void testStreamingExceptions() {
    std::cout << "Testing streaming exceptions..." << std::endl;
    
    // Register mock factory
    DeviceRegistry::registerFactory(RSP1A_HWVER,
        []() { return std::make_unique<MockDeviceControl>(); });
    
    // Create a mock device
    auto device = DeviceRegistry::createDeviceControl(RSP1A_HWVER);
    auto* mockDevice = dynamic_cast<MockDeviceControl*>(device.get());
    assert(mockDevice != nullptr);
    
    // Test streaming without device selected
    bool result = expectException<DeviceException>(
        [&device]() { device->initializeStreaming(); },
        ErrorCode::DEVICE_NOT_SELECTED
    );
    assert(result);
    
    // Simulate device already streaming
    mockDevice->streaming = true;
    
    // Test starting streaming when already streaming
    result = expectException<StreamingException>(
        [&device]() { device->startStreaming(); },
        ErrorCode::STREAMING_ALREADY_ACTIVE
    );
    assert(result);
    
    mockDevice->streaming = false;
    mockDevice->failOnInit = true;
    
    // Test API error during startStreaming
    result = expectException<ApiException>(
        [&device]() { device->startStreaming(); },
        ErrorCode::API_ERROR
    );
    assert(result);
    
    std::cout << "Streaming exceptions test passed" << std::endl;
}

// Test error descriptions
void testErrorDescriptions() {
    std::cout << "Testing error descriptions..." << std::endl;
    
    // Check some error descriptions
    assert(getErrorDescription(ErrorCode::SUCCESS) == "Success");
    assert(getErrorDescription(ErrorCode::DEVICE_NOT_FOUND) == "Device not found");
    assert(getErrorDescription(ErrorCode::STREAMING_ERROR) == "Streaming error");
    assert(getErrorDescription(ErrorCode::UNSUPPORTED_DEVICE) == "Unsupported device hardware version");
    
    // Test full message formatting
    try {
        throw UnsupportedDeviceException("123");
    } catch (const SDRPlayException& e) {
        std::string fullMessage = e.getFullMessage();
        assert(fullMessage.find("Unsupported device hardware version:") != std::string::npos);
        assert(fullMessage.find("123") != std::string::npos);
    }
    
    std::cout << "Error descriptions test passed" << std::endl;
}

int main() {
    try {
        testUnsupportedDevice();
        testDeviceExceptions();
        testStreamingExceptions();
        testErrorDescriptions();
        
        std::cout << "All error handling tests passed" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}