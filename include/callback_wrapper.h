#pragma once
#include <functional>
#include <memory>

namespace sdrplay {

/**
 * @brief A helper class that wraps callback handlers to avoid SWIG binding issues
 * 
 * This class provides a way to register native C++ callbacks while avoiding
 * the issues with SWIG director classes and virtual method overrides.
 */
class CallbackWrapper {
public:
    // Data callback signature: (short* xi, short* xq, unsigned int numSamples)
    using StreamCallbackFn = std::function<void(short*, short*, unsigned int)>;
    
    // Gain callback signature: (int gRdB, int lnaGRdB, float currGain)
    using GainCallbackFn = std::function<void(int, int, float)>;
    
    // Power overload callback signature: (bool isOverloaded)
    using PowerOverloadCallbackFn = std::function<void(bool)>;
    
    CallbackWrapper() = default;
    ~CallbackWrapper() = default;
    
    // Set callbacks
    void setStreamCallback(StreamCallbackFn callback) {
        streamCallback = std::move(callback);
    }
    
    void setGainCallback(GainCallbackFn callback) {
        gainCallback = std::move(callback);
    }
    
    void setPowerOverloadCallback(PowerOverloadCallbackFn callback) {
        powerOverloadCallback = std::move(callback);
    }
    
    // Clear callbacks
    void clearStreamCallback() {
        streamCallback = nullptr;
    }
    
    void clearGainCallback() {
        gainCallback = nullptr;
    }
    
    void clearPowerOverloadCallback() {
        powerOverloadCallback = nullptr;
    }
    
    // Invoke callbacks
    void invokeStreamCallback(short* xi, short* xq, unsigned int numSamples) {
        if (streamCallback) {
            streamCallback(xi, xq, numSamples);
        }
    }
    
    void invokeGainCallback(int gRdB, int lnaGRdB, float currGain) {
        if (gainCallback) {
            gainCallback(gRdB, lnaGRdB, currGain);
        }
    }
    
    void invokePowerOverloadCallback(bool isOverloaded) {
        if (powerOverloadCallback) {
            powerOverloadCallback(isOverloaded);
        }
    }
    
    // Check if callbacks are set
    bool hasStreamCallback() const {
        return streamCallback != nullptr;
    }
    
    bool hasGainCallback() const {
        return gainCallback != nullptr;
    }
    
    bool hasPowerOverloadCallback() const {
        return powerOverloadCallback != nullptr;
    }
    
private:
    StreamCallbackFn streamCallback;
    GainCallbackFn gainCallback;
    PowerOverloadCallbackFn powerOverloadCallback;
};

} // namespace sdrplay