#pragma once
#include "device_types.h"
#include "sdrplay_api.h"
#include "callback_wrapper.h"
#include <memory>
#include <vector>
#include <functional>

namespace sdrplay {

/**
 * @brief Streaming configuration parameters
 */
struct StreamingParams {
    bool enableIQCorrection{true};  // Enable automatic IQ imbalance correction
    bool enableDCCorrection{true};  // Enable automatic DC offset correction
    bool decimate{false};          // Enable decimation
    int decimationFactor{1};       // Decimation factor (1, 2, 4, 8, 16, 32)
    bool wideBandSignal{false};    // Process signal as wideband
    
    // Default constructor
    StreamingParams() = default;
};

class DeviceControl {
public:
    DeviceControl();
    virtual ~DeviceControl();

    // Device management
    virtual bool open();
    virtual void close();
    virtual float getApiVersion() const;
    virtual std::vector<DeviceInfo> getAvailableDevices();
    virtual bool selectDevice(const DeviceInfo& deviceInfo);
    virtual bool releaseDevice();

    // Device access
    virtual sdrplay_api_DeviceT* getCurrentDevice() const;
    virtual sdrplay_api_DeviceParamsT* getDeviceParams() const;
    virtual std::string getLastError() const;

    // Common control methods
    virtual void setFrequency(double freq) = 0;
    virtual double getFrequency() const = 0;
    virtual void setSampleRate(double rate) = 0;
    virtual double getSampleRate() const = 0;

    // RSP1A specific controls
    virtual void setGainReduction(int gain) = 0;
    virtual void setLNAState(int state) = 0;

    // RSPdxR2 specific controls
    virtual void setHDRMode(bool enable) = 0;
    virtual void setBiasTEnabled(bool enable) = 0;
    
    // Streaming methods
    /**
     * @brief Start streaming from the device
     * 
     * @param params Streaming configuration parameters
     * @return true if streaming started successfully
     */
    virtual bool startStreaming(const StreamingParams& params = StreamingParams());
    
    /**
     * @brief Stop streaming from the device
     * 
     * @return true if streaming stopped successfully
     */
    virtual bool stopStreaming();
    
    /**
     * @brief Check if streaming is active
     * 
     * @return true if streaming is active
     */
    virtual bool isStreaming() const;
    
    /**
     * @brief Set the sample callback function
     * 
     * @param callback Function to call with new samples
     */
    virtual void setSampleCallback(CallbackWrapper::SampleCallback callback);
    
    /**
     * @brief Set the event callback function
     * 
     * @param callback Function to call with device events
     */
    virtual void setEventCallback(CallbackWrapper::EventCallback callback);
    
    /**
     * @brief Wait for samples to be available
     * 
     * @param count Number of samples to wait for
     * @param timeoutMs Timeout in milliseconds (0 = no timeout)
     * @return true if samples are available, false on timeout
     */
    virtual bool waitForSamples(size_t count, unsigned int timeoutMs = 0);
    
    /**
     * @brief Read samples from the buffer
     * 
     * @param dest Destination buffer
     * @param maxCount Maximum number of samples to read
     * @return size_t Actual number of samples read
     */
    virtual size_t readSamples(std::complex<short>* dest, size_t maxCount);
    
    /**
     * @brief Get number of available samples
     * 
     * @return size_t Number of samples available
     */
    virtual size_t samplesAvailable() const;
    
    /**
     * @brief Check if buffer overflow occurred
     * 
     * @return true if overflow occurred
     */
    virtual bool hasBufferOverflow() const;
    
    /**
     * @brief Reset buffer state
     */
    virtual void resetBuffer();
    
    /**
     * @brief Get access to the callback wrapper
     * 
     * @return CallbackWrapper* Pointer to the callback wrapper
     */
    virtual CallbackWrapper* getCallbackWrapper();

protected:
    struct Impl;
    std::unique_ptr<Impl> impl;
    
    /**
     * @brief Set up device parameters for streaming
     * 
     * @param params Streaming configuration parameters
     * @return true if parameters set successfully
     */
    virtual bool setupStreamingParameters(const StreamingParams& params);
};

} // namespace sdrplay