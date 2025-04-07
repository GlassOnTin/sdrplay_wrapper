#include "callback_wrapper.h"
#include <algorithm>
#include <chrono>

namespace sdrplay {

//------------------------------------------------------------------------------
// SampleBuffer implementation
//------------------------------------------------------------------------------

SampleBuffer::SampleBuffer(size_t size)
    : buffer(size), readPos(0), writePos(0), overflowed(false) {}

bool SampleBuffer::write(const std::complex<short>* data, size_t count) {
    if (!data || count == 0) {
        return true;
    }

    std::lock_guard<std::mutex> lock(bufferMutex);
    
    // Check if enough space is available
    size_t bufferSize = buffer.size();
    size_t available = (readPos <= writePos) 
        ? bufferSize - (writePos - readPos)
        : readPos - writePos;
    
    // Reserve one slot to differentiate between empty and full buffer
    if (count >= available) {
        overflowed = true;
        return false;
    }
    
    // Write data to buffer
    for (size_t i = 0; i < count; ++i) {
        buffer[writePos] = data[i];
        writePos = (writePos + 1) % bufferSize;
    }
    
    // Notify waiting threads
    dataAvailable.notify_all();
    
    return true;
}

size_t SampleBuffer::read(std::complex<short>* dest, size_t maxCount) {
    if (!dest || maxCount == 0) {
        return 0;
    }

    std::lock_guard<std::mutex> lock(bufferMutex);
    
    if (readPos == writePos) {
        return 0;  // Buffer is empty
    }
    
    size_t bufferSize = buffer.size();
    size_t count = (readPos <= writePos) 
        ? writePos - readPos 
        : bufferSize - readPos + writePos;
    
    count = std::min(count, maxCount);
    
    // Read data from buffer
    for (size_t i = 0; i < count; ++i) {
        dest[i] = buffer[readPos];
        readPos = (readPos + 1) % bufferSize;
    }
    
    return count;
}

bool SampleBuffer::waitForSamples(size_t count, unsigned int timeoutMs) {
    std::unique_lock<std::mutex> lock(bufferMutex);
    
    if (readPos == writePos) {
        // Buffer is empty, wait for data
        if (timeoutMs == 0) {
            // Wait indefinitely
            dataAvailable.wait(lock, [this, count]() {
                size_t available = (readPos <= writePos) 
                    ? writePos - readPos 
                    : buffer.size() - readPos + writePos;
                return available >= count;
            });
            return true;
        } else {
            // Wait with timeout
            auto result = dataAvailable.wait_for(lock, 
                std::chrono::milliseconds(timeoutMs), [this, count]() {
                    size_t available = (readPos <= writePos) 
                        ? writePos - readPos 
                        : buffer.size() - readPos + writePos;
                    return available >= count;
                });
            return result; // true if condition was met, false on timeout
        }
    }
    
    // Check if enough data is available
    size_t available = (readPos <= writePos) 
        ? writePos - readPos 
        : buffer.size() - readPos + writePos;
    
    return available >= count;
}

size_t SampleBuffer::available() const {
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    if (readPos == writePos) {
        return 0;  // Buffer is empty
    }
    
    size_t bufferSize = buffer.size();
    return (readPos <= writePos) 
        ? writePos - readPos 
        : bufferSize - readPos + writePos;
}

bool SampleBuffer::overflow() const {
    return overflowed;
}

void SampleBuffer::reset() {
    std::lock_guard<std::mutex> lock(bufferMutex);
    readPos = 0;
    writePos = 0;
    overflowed = false;
}

size_t SampleBuffer::capacity() const {
    return buffer.size();
}

//------------------------------------------------------------------------------
// CallbackWrapper implementation
//------------------------------------------------------------------------------

CallbackWrapper::CallbackWrapper(size_t bufferSize)
    : sampleBuffer(bufferSize), streamActive(false) {}

CallbackWrapper::~CallbackWrapper() {}

void CallbackWrapper::setSampleCallback(SampleCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex);
    m_sampleCallback = callback;
}

void CallbackWrapper::setEventCallback(EventCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex);
    m_eventCallback = callback;
}

sdrplay_api_StreamCallback_t CallbackWrapper::getStreamCallback() {
    return &CallbackWrapper::streamCallback;
}

sdrplay_api_EventCallback_t CallbackWrapper::getEventCallback() {
    return &CallbackWrapper::eventCallback;
}

bool CallbackWrapper::waitForSamples(size_t count, unsigned int timeoutMs) {
    return sampleBuffer.waitForSamples(count, timeoutMs);
}

size_t CallbackWrapper::readSamples(std::complex<short>* dest, size_t maxCount) {
    return sampleBuffer.read(dest, maxCount);
}

size_t CallbackWrapper::samplesAvailable() const {
    return sampleBuffer.available();
}

bool CallbackWrapper::hasOverflow() const {
    return sampleBuffer.overflow();
}

void CallbackWrapper::resetBuffer() {
    sampleBuffer.reset();
}

void* CallbackWrapper::getContext() {
    return static_cast<void*>(this);
}

void CallbackWrapper::streamCallback(short *xi, short *xq, 
                                    sdrplay_api_StreamCbParamsT *params,
                                    unsigned int numSamples, 
                                    unsigned int reset, 
                                    void *cbContext) {
    // Validate context and cast to CallbackWrapper
    if (!cbContext) {
        return;
    }
    
    CallbackWrapper* wrapper = static_cast<CallbackWrapper*>(cbContext);
    wrapper->processStreamCallback(xi, xq, params, numSamples, reset);
}

void CallbackWrapper::eventCallback(sdrplay_api_EventT eventId,
                                  sdrplay_api_TunerSelectT tuner,
                                  sdrplay_api_EventParamsT *params,
                                  void *cbContext) {
    // Validate context and cast to CallbackWrapper
    if (!cbContext) {
        return;
    }
    
    CallbackWrapper* wrapper = static_cast<CallbackWrapper*>(cbContext);
    wrapper->processEventCallback(eventId, tuner, params);
}

void CallbackWrapper::processStreamCallback(short *xi, short *xq, 
                                          sdrplay_api_StreamCbParamsT *params,
                                          unsigned int numSamples, 
                                          unsigned int reset) {
    // Handle reset condition
    if (reset) {
        resetBuffer();
        streamActive = true;
    }
    
    if (!streamActive) {
        return;
    }
    
    // Convert separate I/Q arrays to complex samples
    std::vector<std::complex<short>> samples(numSamples);
    for (unsigned int i = 0; i < numSamples; ++i) {
        samples[i] = std::complex<short>(xi[i], xq[i]);
    }
    
    // Write samples to buffer
    bool success = sampleBuffer.write(samples.data(), numSamples);
    
    // Call user callback if provided
    std::lock_guard<std::mutex> lock(callbackMutex);
    if (m_sampleCallback) {
        m_sampleCallback(samples.data(), numSamples);
    }
}

void CallbackWrapper::processEventCallback(sdrplay_api_EventT eventId,
                                         sdrplay_api_TunerSelectT tuner,
                                         sdrplay_api_EventParamsT *params) {
    EventType type = EventType::None;
    EventParams eventParams;
    
    // Map SDRplay event to our event type and extract parameters
    switch (eventId) {
        case sdrplay_api_GainChange:
            type = EventType::GainChange;
            if (params) {
                eventParams.gRdB = params->gainParams.gRdB;
                eventParams.lnaGRdB = params->gainParams.lnaGRdB;
                eventParams.currGain = params->gainParams.currGain;
            }
            break;
            
        case sdrplay_api_PowerOverloadChange:
            type = EventType::PowerOverload;
            if (params) {
                eventParams.overloadDetected = 
                    (params->powerOverloadParams.powerOverloadChangeType == 
                     sdrplay_api_Overload_Detected);
            }
            break;
            
        case sdrplay_api_DeviceRemoved:
            type = EventType::DeviceRemoved;
            eventParams.deviceRemoved = 1;
            streamActive = false;
            break;
            
        case sdrplay_api_RspDuoModeChange:
            type = EventType::RspDuoModeChange;
            break;
            
        default:
            type = EventType::None;
            break;
    }
    
    // Call user callback if provided
    std::lock_guard<std::mutex> lock(callbackMutex);
    if (m_eventCallback) {
        m_eventCallback(type, eventParams);
    }
}

} // namespace sdrplay