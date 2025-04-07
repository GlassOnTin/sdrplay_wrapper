#pragma once
#include <functional>
#include <memory>
#include <vector>
#include <complex>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "sdrplay_api.h"

namespace sdrplay {

/**
 * @brief Enumeration of event types that can be received from the device
 */
enum class EventType {
    GainChange,
    PowerOverload,
    DeviceRemoved,
    ADCOverflow,
    RspDuoModeChange,
    None
};

/**
 * @brief Parameters for events
 */
struct EventParams {
    int gRdB;             // Gain reduction in dB
    int lnaGRdB;          // LNA gain reduction in dB
    float currGain;       // Current system gain
    bool overloadDetected; // Power overload detected
    int deviceRemoved;    // Device removed

    EventParams() : gRdB(0), lnaGRdB(0), currGain(0.0f), 
                    overloadDetected(false), deviceRemoved(0) {}
};

/**
 * @brief Buffer for streaming samples
 * 
 * Thread-safe circular buffer to store IQ samples
 */
class SampleBuffer {
public:
    /**
     * @brief Construct a new Sample Buffer object
     * 
     * @param size Buffer size in number of complex samples
     */
    SampleBuffer(size_t size);
    
    /**
     * @brief Write samples to buffer
     * 
     * @param data Complex sample data to write
     * @param count Number of samples to write
     * @return true if successful, false if buffer overflow
     */
    bool write(const std::complex<short>* data, size_t count);
    
    /**
     * @brief Read samples from buffer
     * 
     * @param dest Destination buffer
     * @param maxCount Maximum number of samples to read
     * @return size_t Actual number of samples read
     */
    size_t read(std::complex<short>* dest, size_t maxCount);
    
    /**
     * @brief Wait for samples to be available
     * 
     * @param count Number of samples to wait for
     * @param timeoutMs Timeout in milliseconds (0 = no timeout)
     * @return true if samples are available, false on timeout
     */
    bool waitForSamples(size_t count, unsigned int timeoutMs = 0);
    
    /**
     * @brief Get number of samples available for reading
     * 
     * @return size_t Number of samples available
     */
    size_t available() const;
    
    /**
     * @brief Check if buffer overflow occurred
     * 
     * @return true if overflow occurred
     */
    bool overflow() const;
    
    /**
     * @brief Reset buffer state
     */
    void reset();
    
    /**
     * @brief Get buffer capacity
     * 
     * @return size_t Buffer capacity in samples
     */
    size_t capacity() const;

private:
    std::vector<std::complex<short>> buffer;
    size_t readPos;
    size_t writePos;
    std::atomic<bool> overflowed;
    mutable std::mutex bufferMutex;
    std::condition_variable dataAvailable;
};

/**
 * @brief Wrapper for SDRPlay API callbacks
 * 
 * This class manages streaming data and event callbacks from the SDRplay API.
 */
class CallbackWrapper {
public:
    /**
     * @brief User callback type for IQ samples
     */
    using SampleCallback = std::function<void(const std::complex<short>*, size_t)>;
    
    /**
     * @brief User callback type for device events
     */
    using EventCallback = std::function<void(EventType, const EventParams&)>;
    
    /**
     * @brief Construct a new CallbackWrapper
     * 
     * @param bufferSize Size of the internal sample buffer
     */
    CallbackWrapper(size_t bufferSize = 262144); // Default to 256K samples
    
    /**
     * @brief Destructor
     */
    ~CallbackWrapper();
    
    /**
     * @brief Set the sample callback function
     * 
     * This function will be called when new samples are available
     * 
     * @param callback Function to call with new samples
     */
    void setSampleCallback(SampleCallback callback);
    
    /**
     * @brief Set the event callback function
     * 
     * This function will be called when device events occur
     * 
     * @param callback Function to call with events
     */
    void setEventCallback(EventCallback callback);
    
    /**
     * @brief Get SDRplay API stream callback function
     * 
     * @return Function pointer to stream callback
     */
    sdrplay_api_StreamCallback_t getStreamCallback();
    
    /**
     * @brief Get SDRplay API event callback function
     * 
     * @return Function pointer to event callback
     */
    sdrplay_api_EventCallback_t getEventCallback();
    
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
     * @param dest Destination buffer
     * @param maxCount Maximum number of samples to read
     * @return size_t Actual number of samples read
     */
    size_t readSamples(std::complex<short>* dest, size_t maxCount);
    
    /**
     * @brief Get number of available samples
     * 
     * @return size_t Number of samples available
     */
    size_t samplesAvailable() const;
    
    /**
     * @brief Check if buffer overflow occurred
     * 
     * @return true if overflow occurred
     */
    bool hasOverflow() const;
    
    /**
     * @brief Reset buffer state
     */
    void resetBuffer();
    
    /**
     * @brief Get a pointer to the internal context
     * 
     * @return void* Pointer to this object for use in callbacks
     */
    void* getContext();
    
    /**
     * @brief Static callback handler for stream data
     * 
     * This is the function passed to the SDRplay API
     * 
     * @param xi I samples
     * @param xq Q samples
     * @param params Stream callback parameters
     * @param numSamples Number of samples
     * @param reset Reset flag
     * @param cbContext Callback context (should be CallbackWrapper*)
     */
    static void streamCallback(short *xi, short *xq, 
                              sdrplay_api_StreamCbParamsT *params,
                              unsigned int numSamples, 
                              unsigned int reset, 
                              void *cbContext);
    
    /**
     * @brief Static callback handler for events
     * 
     * This is the function passed to the SDRplay API
     * 
     * @param eventId Event ID
     * @param tuner Tuner select
     * @param params Event parameters
     * @param cbContext Callback context (should be CallbackWrapper*)
     */
    static void eventCallback(sdrplay_api_EventT eventId,
                            sdrplay_api_TunerSelectT tuner,
                            sdrplay_api_EventParamsT *params,
                            void *cbContext);
    
private:
    /**
     * @brief Process a stream callback
     * 
     * @param xi I samples
     * @param xq Q samples
     * @param params Stream callback parameters
     * @param numSamples Number of samples
     * @param reset Reset flag
     */
    void processStreamCallback(short *xi, short *xq, 
                               sdrplay_api_StreamCbParamsT *params,
                               unsigned int numSamples, 
                               unsigned int reset);
    
    /**
     * @brief Process an event callback
     * 
     * @param eventId Event ID
     * @param tuner Tuner select
     * @param params Event parameters
     */
    void processEventCallback(sdrplay_api_EventT eventId,
                              sdrplay_api_TunerSelectT tuner,
                              sdrplay_api_EventParamsT *params);
    
    SampleCallback m_sampleCallback;
    EventCallback m_eventCallback;
    SampleBuffer sampleBuffer;
    std::mutex callbackMutex;
    std::atomic<bool> streamActive;
};

} // namespace sdrplay