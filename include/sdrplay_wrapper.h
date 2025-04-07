// include/sdrplay_wrapper.h
#pragma once
#include <memory>
#include <vector>
#include <complex>
#include "device_types.h"
#include "callback_wrapper.h"

namespace sdrplay {

// Forward declarations
class RSP1AParameters;
class RSPdxR2Parameters;
using Rsp1aParams = RSP1AParameters; 
using RspDxR2Params = RSPdxR2Parameters;

/**
 * @brief Main device class for SDRplay API
 * 
 * This class provides the primary interface for interacting with SDRplay devices.
 */
class Device {
public:
    /**
     * @brief Default constructor
     */
    Device();
    
    /**
     * @brief Destructor
     */
    ~Device();

    // Disable copy, default move
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    Device(Device&&) = default;
    Device& operator=(Device&&) = default;

    /**
     * @brief Select a specific device
     * 
     * @param deviceInfo Device to select
     * @return true if successful
     */
    bool selectDevice(const DeviceInfo& deviceInfo);
    
    /**
     * @brief Release the currently selected device
     * 
     * @return true if successful
     */
    bool releaseDevice();
    
    /**
     * @brief Get list of available devices
     * 
     * @return std::vector<DeviceInfo> List of devices
     */
    std::vector<DeviceInfo> getAvailableDevices();

    /**
     * @brief Set frequency in Hz
     * 
     * @param freq Frequency in Hz
     */
    void setFrequency(double freq);
    
    /**
     * @brief Get current frequency in Hz
     * 
     * @return double Frequency in Hz
     */
    double getFrequency() const;
    
    /**
     * @brief Set sample rate in Hz
     * 
     * @param rate Sample rate in Hz
     */
    void setSampleRate(double rate);
    
    /**
     * @brief Get current sample rate in Hz
     * 
     * @return double Sample rate in Hz
     */
    double getSampleRate() const;
    
    /**
     * @brief Access RSP1A parameters if device is RSP1A
     * 
     * @return Rsp1aParams* Pointer to RSP1A parameters or nullptr
     */
    Rsp1aParams* getRsp1aParams();
    
    /**
     * @brief Access RSPdxR2 parameters if device is RSPdxR2
     * 
     * @return RspDxR2Params* Pointer to RSPdxR2 parameters or nullptr
     */
    RspDxR2Params* getRspDxR2Params();
    
    /**
     * @brief Start streaming from the device
     * 
     * @param enableDcCorrection Enable DC offset correction
     * @param enableIqCorrection Enable IQ imbalance correction
     * @param decimationFactor Decimation factor (1, 2, 4, 8, 16, 32)
     * @return true if streaming started successfully
     */
    bool startStreaming(bool enableDcCorrection = true, 
                       bool enableIqCorrection = true,
                       int decimationFactor = 1);
    
    /**
     * @brief Stop streaming from the device
     * 
     * @return true if streaming stopped successfully
     */
    bool stopStreaming();
    
    /**
     * @brief Check if streaming is active
     * 
     * @return true if streaming is active
     */
    bool isStreaming() const;
    
    /**
     * @brief Set callback for samples
     * 
     * @param callback Function to call when samples are received
     */
    void setSampleCallback(std::function<void(const std::complex<short>*, size_t)> callback);
    
    /**
     * @brief Set callback for events
     * 
     * @param callback Function to call when events occur
     */
    void setEventCallback(std::function<void(EventType, const EventParams&)> callback);
    
    /**
     * @brief Wait for samples to be available
     * 
     * @param count Number of samples to wait for
     * @param timeoutMs Timeout in milliseconds (0 = no timeout)
     * @return true if samples are available, false on timeout
     */
    bool waitForSamples(size_t count, unsigned int timeoutMs = 0);
    
    /**
     * @brief Read samples from the buffer
     * 
     * @param buffer Destination buffer
     * @param maxCount Maximum number of samples to read
     * @return size_t Actual number of samples read
     */
    size_t readSamples(std::complex<short>* buffer, size_t maxCount);
    
    /**
     * @brief Get number of samples available in buffer
     * 
     * @return size_t Number of samples available
     */
    size_t samplesAvailable() const;
    
    /**
     * @brief Check if buffer overflow occurred
     * 
     * @return true if overflow occurred
     */
    bool hasBufferOverflow() const;
    
    /**
     * @brief Reset buffer state
     */
    void resetBuffer();

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace sdrplay