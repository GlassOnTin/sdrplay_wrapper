# SDRplay Wrapper Streaming Implementation Plan

This document outlines the plan for implementing streaming functionality in the SDRplay C++ wrapper and Python bindings.

## Overview

The SDRplay API provides streaming capabilities through callback functions. The current C++ wrapper library has placeholder structures but no actual implementation. This document outlines the steps needed to implement streaming functionality in both the C++ wrapper and Python bindings.

## Current Status

- The SDRplay native C API provides streaming via callbacks
- The C++ wrapper has a placeholder `CallbackWrapper` class with no implementation
- No streaming functionality is exposed to Python bindings

## Implementation Phases

### Phase 1: C++ Callback Implementation

1. **Enhanced `CallbackWrapper` Class**
   - Implement the `CallbackWrapper` class to manage SDRplay API callbacks
   - Create methods to register user-provided callback functions
   - Add buffer management for sample data
   - Implement thread safety mechanisms

2. **Stream Configuration**
   - Add streaming configuration parameters to device classes
   - Implement buffer size and queue depth configuration
   - Add sample format options

3. **Device Control Extensions**
   - Extend `DeviceControl` class to handle streaming initialization
   - Add streaming start/stop methods
   - Implement error handling for streaming operations

### Phase 2: C++ Stream Processing

1. **Buffer Management**
   - Implement circular buffer for sample data
   - Create thread-safe producer/consumer queue
   - Add overflow detection and handling

2. **Sample Processing Pipeline**
   - Implement basic signal processing capabilities (optional)
   - Add filtering options
   - Create sample converter utilities (complex to real, etc.)

3. **Streaming Status Management**
   - Implement status tracking for streaming
   - Add statistics collection (dropped samples, etc.)
   - Create error reporting mechanism

### Phase 3: Python Bindings for Streaming

1. **SWIG Interface Extensions**
   - Extend `sdrplay.i` to expose streaming interfaces
   - Create Python-friendly callback mechanisms
   - Implement NumPy array integration for efficient data handling

2. **Python Streaming API**
   - Design Pythonic API for streaming control
   - Implement Python callback registration
   - Create Python buffer objects for stream data

3. **Python Stream Processing Tools**
   - Create helper classes for common signal processing tasks
   - Implement NumPy-based processing utilities
   - Add visualization helpers (optional)

## Technical Implementation Details

### C++ Callback Implementation

```cpp
// Enhanced CallbackWrapper Class
class CallbackWrapper {
public:
    // User callback function type
    using SampleCallback = std::function<void(const std::complex<short>*, size_t)>;
    using EventCallback = std::function<void(EventType, const EventParams&)>;
    
    // Set callbacks
    void setSampleCallback(SampleCallback callback);
    void setEventCallback(EventCallback callback);
    
    // Internal callback handlers (called by SDRplay API)
    static void streamCallback(short *xi, short *xq, 
                              sdrplay_api_StreamCbParamsT *params,
                              unsigned int numSamples, 
                              unsigned int reset, 
                              void *cbContext);
                              
    static void eventCallback(sdrplay_api_EventT eventId,
                             sdrplay_api_TunerSelectT tuner,
                             sdrplay_api_EventParamsT *params,
                             void *cbContext);

private:
    SampleCallback sampleCallback;
    EventCallback eventCallback;
    std::vector<std::complex<short>> sampleBuffer;
    std::mutex callbackMutex;
};
```

### Buffer Management

```cpp
// Thread-safe buffer for stream data
class SampleBuffer {
public:
    SampleBuffer(size_t size);
    
    // Producer methods (called from callback)
    bool write(const std::complex<short>* data, size_t count);
    
    // Consumer methods (called from user code)
    size_t read(std::complex<short>* dest, size_t maxCount);
    
    // Status methods
    size_t available() const;
    bool overflow() const;
    void reset();
    
private:
    std::vector<std::complex<short>> buffer;
    size_t readPos;
    size_t writePos;
    std::atomic<bool> overflowed;
    mutable std::mutex bufferMutex;
    std::condition_variable dataAvailable;
};
```

### Python Bindings

```python
# Python streaming API example
class StreamCallbackHandler:
    def __init__(self, buffer_size=8192):
        self.buffer = np.zeros(buffer_size, dtype=np.complex64)
        
    def callback(self, samples, count):
        # Convert C++ samples to NumPy array
        # Process data
        pass
        
# Usage example
device = sdrplay.Device()
device.selectDevice(device_info)

# Set up streaming
handler = StreamCallbackHandler()
device.setSampleCallback(handler.callback)
device.setStreamingParams(sample_rate=2.048e6, buffer_size=8192)

# Start/stop streaming
device.startStreaming()
# ... do processing ...
device.stopStreaming()
```

## SWIG Interface Requirements

Add to `sdrplay.i`:

```
// Handle complex numbers
%include "std_complex.i"

// Handle callbacks
%feature("director") CallbackHandler;

// Enable NumPy support
%include "numpy.i"
%init %{
    import_array();
%}

// Define callback handler class
%inline %{
class CallbackHandler {
public:
    CallbackHandler() {}
    virtual ~CallbackHandler() {}
    virtual void handleSamples(const std::complex<short>* samples, size_t count) {
        // Default implementation does nothing
    }
    virtual void handleEvent(int eventType, void* params) {
        // Default implementation does nothing
    }
};
%}
```

## Implementation Timeline

1. **Phase 1: C++ Callback Implementation** - 2-3 weeks
   - Week 1: Design and implement core callback structures
   - Week 2: Implement buffer management and thread safety
   - Week 3: Testing and debugging

2. **Phase 2: C++ Stream Processing** - 2-3 weeks
   - Week 1: Implement buffer management
   - Week 2: Implement sample processing pipeline
   - Week 3: Testing and optimization

3. **Phase 3: Python Bindings** - 2-3 weeks
   - Week 1: Extend SWIG interface
   - Week 2: Implement Python callback mechanisms
   - Week 3: Testing and documentation

## Conclusion

Implementing streaming functionality requires careful consideration of thread safety, buffer management, and callback handling. The plan outlined above provides a structured approach to adding this functionality to both the C++ wrapper and Python bindings.

Start with the C++ implementation first, ensuring it's robust and efficient before exposing it through Python bindings. Use SWIG's director feature to allow Python callbacks to be invoked from C++ code.

This implementation will enable real-time streaming applications such as FM radio receivers, spectrum analyzers, and digital signal processing applications to be built using the SDRplay wrapper.