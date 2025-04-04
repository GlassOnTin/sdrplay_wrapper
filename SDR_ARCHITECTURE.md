# SDRPlay Wrapper Architecture

This document outlines the architecture and design decisions for the SDRPlay wrapper library, focusing on the streaming components and overall system design.

## Overview

The SDRPlay wrapper is a C++ library with Python bindings that provides a simple and efficient interface to SDRPlay SDR devices. The library is designed to be:

- **Easy to use**: Simple API that abstracts away the complexity of the SDRPlay API
- **Flexible**: Support for multiple device types and configurations
- **Efficient**: Optimized for real-time signal processing
- **Extensible**: Easy to add support for new devices and features

## Core Components

### 1. Device Access Layer

The core of the wrapper is the device access layer, which handles communication with the SDRPlay API:

- `DeviceControl`: Abstract base class for device control
- `RSP1AControl`: Implementation for RSP1A devices
- `RSPdxR2Control`: Implementation for RSPdx-R2 devices
- `DeviceRegistry`: Factory for creating device implementations

### 2. Parameter Management

Device parameters are managed through specialized classes:

- `BasicParams`: Common parameters across all devices
- `ControlParams`: Control parameters for all devices
- `RSP1AParameters`: RSP1A-specific parameters
- `RSPdxR2Parameters`: RSPdx-R2-specific parameters

### 3. Error Handling

Consistent error handling is provided through a custom exception hierarchy:

- `SDRPlayException`: Base exception class
- `ApiException`: For SDRPlay API errors
- `DeviceException`: For device-related errors
- `StreamingException`: For streaming-related errors
- `ParameterException`: For parameter-related errors
- `UnsupportedDeviceException`: For unsupported device types

### 4. Callback System

Streaming data and events are handled through a callback system:

- `StreamCallbackHandler`: Interface for receiving streaming data
- `GainCallbackHandler`: Interface for gain change notifications
- `PowerOverloadCallbackHandler`: Interface for power overload notifications
- `CallbackWrapper`: Efficient wrapper for callback handlers

### 5. Python Bindings

Python bindings are provided through SWIG:

- `sdrplay.py`: Python module for accessing the wrapper
- `fm_radio.py`: High-level FM radio implementation
- Custom typemaps for handling arrays and callbacks

## Streaming Architecture

### Streaming Flow

1. **Device Initialization**:
   - User creates a `Device` object
   - Device is selected and configured
   - Callbacks are registered

2. **Stream Setup**:
   - User registers callback handlers
   - Streaming parameters are configured

3. **Stream Start**:
   - User calls `startStreaming()`
   - SDRPlay API is set up for streaming
   - Callbacks begin delivering data

4. **Data Processing**:
   - Data is received in the stream callback
   - User processes data in their callback handler
   - Audio or other processing is performed

5. **Stream Stop**:
   - User calls `stopStreaming()`
   - SDRPlay API streaming is stopped
   - Callbacks cease delivering data

### Callback Implementation

Due to SWIG binding limitations, we use a dual approach for callbacks:

1. **C++ Side**:
   - Static callback functions that receive data from the SDRPlay API
   - Mapping from device handles to device instances
   - Thread-safe callback invocation

2. **Python Side**:
   - Simple function-based callbacks that avoid SWIG "director" classes
   - Direct function calls through the `CallbackWrapper`
   - Efficient typemaps for array conversion

## High-Level Applications

On top of the core wrapper, we provide high-level applications:

1. **FMRadio**:
   - FM demodulation and audio output
   - Signal level measurement
   - Tuning and mode selection

2. **Command-Line Interface**:
   - Interactive terminal UI
   - Station scanning and presets
   - Audio controls

## Memory Management

Memory management is handled through smart pointers:

- `std::unique_ptr` for ownership of device implementations
- `std::shared_ptr` for shared resources
- RAII pattern for resource management

## Testing

The library includes comprehensive testing:

- Unit tests for individual components
- Integration tests for the full API
- Test fixtures for device simulation

## Future Improvements

1. **Enhanced Callback System**:
   - Improve Python callback integration
   - Add more flexible callback parameters

2. **DSP Library Integration**:
   - Add built-in DSP functions
   - Support for common signal processing operations

3. **Additional Device Support**:
   - Support for more SDRPlay devices
   - Support for other SDR hardware

4. **Performance Optimization**:
   - Zero-copy buffer management
   - SIMD optimizations for DSP operations

## Usage Examples

### Basic Device Usage
```cpp
// C++ example
auto device = std::make_unique<sdrplay::Device>();
auto devices = device->getAvailableDevices();
if (!devices.empty()) {
    device->selectDevice(devices[0]);
    device->setFrequency(100e6);  // 100 MHz
    device->setSampleRate(2e6);   // 2 MHz sample rate
    // Use the device...
    device->releaseDevice();
}
```

```python
# Python example
import sdrplay
device = sdrplay.Device()
devices = device.getAvailableDevices()
if len(devices) > 0:
    device.selectDevice(devices[0])
    device.setFrequency(100e6)  # 100 MHz
    device.setSampleRate(2e6)   # 2 MHz sample rate
    # Use the device...
    device.releaseDevice()
```

### Streaming Example
```cpp
// C++ streaming example
class MyStreamHandler : public sdrplay::StreamCallbackHandler {
public:
    void handleStreamData(short* xi, short* xq, unsigned int numSamples) override {
        // Process data...
    }
};

auto device = std::make_unique<sdrplay::Device>();
auto handler = std::make_unique<MyStreamHandler>();
device->registerStreamCallback(handler.get());
device->startStreaming();
// ...
device->stopStreaming();
```

```python
# Python streaming example
import sdrplay
import numpy as np

class StreamHandler(sdrplay.StreamCallbackHandler):
    def handleStreamData(self, xi, xq, numSamples):
        # xi and xq are numpy arrays
        complex_samples = xi + 1j * xq
        # Process data...

device = sdrplay.Device()
handler = StreamHandler()
device.registerStreamCallback(handler)
device.startStreaming()
# ...
device.stopStreaming()
```

### FM Radio Example
```python
# Python FM radio example
from sdrplay.fm_radio import FMRadio

radio = FMRadio()
radio.connect()
radio.tune(100e6)  # 100 MHz
radio.set_demod_mode("FM")
radio.start()
# Radio plays audio...
radio.stop()
radio.close()
```

## Conclusion

The SDRPlay wrapper provides a powerful and flexible interface to SDRPlay devices, with a focus on ease of use and performance. The architecture is designed to be extensible and maintainable, with clear separation of concerns and a consistent API.

The streaming architecture in particular is designed to balance performance with ease of use, providing efficient data processing while maintaining a simple and intuitive API. The callback system provides a flexible way to receive and process streaming data, while the high-level applications provide ready-to-use functionality for common SDR tasks.