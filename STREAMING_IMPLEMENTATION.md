# SDRplay Wrapper Streaming Implementation

This document describes the streaming implementation in the SDRplay C++ wrapper and Python bindings.

## Overview

The SDRplay wrapper now includes full streaming support for SDRplay devices through the native SDRplay API. This allows applications to:

1. Receive continuous IQ data streams from SDRplay devices
2. Process data through callbacks or direct buffer reading
3. Integrate with NumPy for Python data processing
4. Build applications like spectrum analyzers and FM receivers

## C++ Implementation

### Key Components

1. **CallbackWrapper**: Manages callbacks from the SDRplay API and provides a high-level C++ interface
   - Handles SDRplay API's native IQ callback format
   - Converts separate I/Q arrays to std::complex
   - Manages threading and synchronization
   - Provides both callback-based and polling-based interfaces

2. **SampleBuffer**: Thread-safe circular buffer for IQ samples
   - Efficiently stores received samples
   - Handles buffer overflow conditions
   - Provides thread synchronization for producer/consumer model
   - Optimized for high-throughput streaming

3. **StreamingParams**: Configuration structure for streaming
   - Controls DC offset correction
   - Controls IQ imbalance correction
   - Supports decimation settings
   - Configures other stream parameters

4. **DeviceControl Extensions**: Streaming functionality for device control
   - Manages streaming lifecycle (start/stop)
   - Configures stream parameters
   - Provides a higher-level API for streaming

5. **Device Extensions**: High-level streaming interface
   - Easy-to-use streaming API for applications
   - Abstracts hardware-specific details
   - Provides simple configuration options

### Class Relationships

- **Device** uses **DeviceControl** for low-level device operations
- **DeviceControl** manages **CallbackWrapper** to handle callbacks
- **CallbackWrapper** uses **SampleBuffer** for sample storage and retrieval
- Applications can use either Device's high-level API or access lower-level components directly

## Python Bindings

### Key Features

1. **NumPy Integration**
   - IQ samples can be directly accessed as NumPy arrays
   - Enables efficient signal processing with NumPy/SciPy
   - Avoids copying data where possible

2. **Callback Mechanism**
   - Python callback handlers can receive and process samples
   - SWIG director feature enables C++ to call Python methods
   - Callback mechanism supports complex sample data as NumPy arrays

3. **Direct Buffer Reading**
   - Allows polling for available samples
   - Supports direct reading into NumPy arrays
   - Useful for integration with existing processing pipelines

## Usage Examples

### C++ Example (Callback-based)

```cpp
#include "sdrplay_wrapper.h"
#include <iostream>
#include <vector>
#include <complex>

// Callback function for samples
void processSamples(const std::complex<short>* samples, size_t count) {
    std::cout << "Received " << count << " samples" << std::endl;
    // Process samples here...
}

int main() {
    sdrplay::Device device;
    auto devices = device.getAvailableDevices();
    
    if (devices.empty()) {
        std::cerr << "No devices found" << std::endl;
        return 1;
    }
    
    if (!device.selectDevice(devices[0])) {
        std::cerr << "Failed to select device" << std::endl;
        return 1;
    }
    
    // Configure device
    device.setFrequency(100e6); // 100 MHz
    device.setSampleRate(2e6);  // 2 MHz
    
    // Set up callback
    device.setSampleCallback(processSamples);
    
    // Start streaming
    if (!device.startStreaming()) {
        std::cerr << "Failed to start streaming" << std::endl;
        return 1;
    }
    
    // Stream for a while
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // Stop streaming
    device.stopStreaming();
    device.releaseDevice();
    
    return 0;
}
```

### Python Example (Callback-based)

```python
import numpy as np
from sdrplay import *

class MySampleHandler(SampleCallbackHandler):
    def __init__(self):
        SampleCallbackHandler.__init__(self)
        self.samples = []
        
    def handleSamples(self, samples, count):
        # Convert samples to NumPy array
        array_slice = np.array([complex(samples[i].real, samples[i].imag) 
                      for i in range(count)])
        self.samples.extend(array_slice)
        print(f"Received {count} samples")

# Initialize the device registry
initializeDeviceRegistry()

# Create device
device = Device()
devices = device.getAvailableDevices()
if devices:
    device.selectDevice(devices[0])
    device.setFrequency(100e6)
    device.setSampleRate(2e6)
    
    # Create and set up the sample handler
    handler = MySampleHandler()
    device.setPythonSampleCallback(handler)
    
    # Start streaming
    device.startStreaming()
    
    # Stream for 5 seconds
    import time
    time.sleep(5)
    
    # Stop streaming
    device.stopStreaming()
    device.setPythonSampleCallback(None)
    device.releaseDevice()
    
    print(f"Collected {len(handler.samples)} samples")
```

### Python Example (Direct Read)

```python
import numpy as np
from sdrplay import *

# Initialize and configure device
initializeDeviceRegistry()
device = Device()
devices = device.getAvailableDevices()
if devices:
    device.selectDevice(devices[0])
    device.setFrequency(100e6)
    device.setSampleRate(2e6)
    
    # Start streaming
    device.startStreaming()
    
    # Read samples directly
    import time
    start_time = time.time()
    all_samples = np.array([], dtype=np.complex64)
    
    while time.time() - start_time < 5:  # Read for 5 seconds
        # Check if samples are available
        available = device.samplesAvailable()
        
        if available > 0:
            # Read samples directly to a NumPy array
            samples = device.readSamplesToNumpy(available)
            all_samples = np.append(all_samples, samples)
            
        time.sleep(0.01)
    
    # Stop streaming and release the device
    device.stopStreaming()
    device.releaseDevice()
    
    print(f"Collected {len(all_samples)} samples")
```

## Applications

The streaming implementation enables a variety of applications:

1. **Spectrum Analyzers**: Real-time signal analysis and visualization
2. **FM Radio Receivers**: Demodulate FM signals for audio output
3. **Digital Signal Processing**: Apply DSP algorithms to received signals
4. **SDR Applications**: Build custom SDR applications with Python
5. **Data Recording**: Save IQ data for later processing
6. **Signal Analysis**: Analyze signals with NumPy/SciPy tools

## Future Enhancements

Potential future enhancements to the streaming implementation:

1. **Zero-copy NumPy Integration**: Further optimize Python integration
2. **Additional Processing Pipelines**: Add more signal processing components
3. **Performance Optimizations**: Improve buffer handling for high-rate streaming
4. **Multi-Device Support**: Support for streaming from multiple devices
5. **Stream Format Options**: Support for different data formats and precision

## Conclusion

The streaming implementation provides a complete solution for receiving and processing IQ data from SDRplay devices in both C++ and Python. It balances ease of use with performance, making it suitable for a wide range of applications from simple experiments to complex signal processing systems.