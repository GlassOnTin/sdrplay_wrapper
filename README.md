# SDRPlay Python Wrapper

This is a Python wrapper for the SDRPlay API, supporting RSP1A and RSPdx-R2 devices.

## Requirements

- SDRPlay API v3.0 or newer
- C++17 compatible compiler
- SWIG 4.0 or newer
- CMake 3.12 or newer
- Python 3.8 or newer

## Building

```bash
mkdir -p build && cd build
cmake ..
make
```

## Recent Updates

- Added streaming API support with callback registration
- Fixed device registry and implemented clearFactories method
- Fixed Python bindings for RSPdxR2 device support
- Added device detection and basic parameter control
- Updated SWIG interface for better Python integration
- Added debug logging to help diagnose device connection issues

## Usage

### Basic Usage

```python
import sdrplay

# Create a device
device = sdrplay.Device()

# Get available devices
devices = device.getAvailableDevices()
print(f"Found {len(devices)} devices")

# If we found a device, select it
if len(devices) > 0:
    device.selectDevice(devices[0])
    
    # Set frequency and sample rate
    device.setFrequency(100e6)  # 100 MHz
    device.setSampleRate(2e6)   # 2 MHz
    
    # Get device-specific parameters
    if device_info.hwVer == sdrplay.RSPDXR2_HWVER:
        rspdxr2_params = device.getRspDxR2Params()
        # Configure RSPdxR2-specific features
    
    elif device_info.hwVer == sdrplay.RSP1A_HWVER:
        rsp1a_params = device.getRsp1aParams()
        # Configure RSP1A-specific features
        
    # When done
    device.releaseDevice()
```

### Streaming API

The SDRPlay wrapper provides a callback-based streaming API that allows you to receive IQ samples and handle events from the device. The key callback interfaces are:

- **StreamCallbackHandler**: Receives IQ sample data from the device
- **GainCallbackHandler**: Notified when gain changes occur
- **PowerOverloadCallbackHandler**: Notified when power overload conditions occur

Here's a simple example:

```python
import sdrplay
import numpy as np
import time

# Define your callback handlers
class StreamHandler(sdrplay.StreamCallbackHandler):
    def __init__(self):
        # Note: For abstract classes in SWIG, don't call the parent constructor
        self.buffer = []
        self.total_samples = 0
    
    def handleStreamData(self, xi, xq, numSamples):
        # Convert to numpy arrays for easier processing
        i_data = np.array(xi[:numSamples])
        q_data = np.array(xq[:numSamples])
        
        # Process IQ samples here
        print(f"Received {numSamples} samples")
        self.total_samples += numSamples
        
        # Store some samples for later analysis
        if len(self.buffer) < 5:
            complex_samples = i_data[:10] + 1j * q_data[:10]
            self.buffer.append(complex_samples)

class GainHandler(sdrplay.GainCallbackHandler):
    def handleGainChange(self, gRdB, lnaGRdB, currGain):
        print(f"Gain change: {gRdB} dB, LNA: {lnaGRdB} dB, Current: {currGain}")

class PowerHandler(sdrplay.PowerOverloadCallbackHandler):
    def handlePowerOverload(self, isOverloaded):
        print(f"Power overload detected: {isOverloaded}")

# Create device
device = sdrplay.Device()
devices = device.getAvailableDevices()
if len(devices) > 0:
    device.selectDevice(devices[0])
    
    # Set frequency and sample rate
    device.setFrequency(100e6)
    device.setSampleRate(2e6)
    
    # Register callbacks
    stream_cb = StreamHandler()
    gain_cb = GainHandler()
    power_cb = PowerHandler()
    
    device.registerStreamCallback(stream_cb)
    device.registerGainCallback(gain_cb)
    device.registerPowerOverloadCallback(power_cb)
    
    # Start streaming
    if device.startStreaming():
        print("Streaming started")
        
        # Process data for 5 seconds
        time.sleep(5)
        
        # Stop streaming
        device.stopStreaming()
        print("Streaming stopped")
    
    # When done
    device.releaseDevice()
```

### Included Example

The repository includes a more complete example script `example_streaming.py` that demonstrates streaming with:

- Command-line argument parsing for frequency, sample rate, and gain settings
- Clean signal handling for graceful termination
- Basic signal statistics calculation
- Device-specific parameter configuration
- Full error handling and logging

You can run it with:

```bash
python3 example_streaming.py --freq 100.0 --samplerate 2.0 --gain 40 --time 10
```

## Testing

```bash
# C++ tests
cd build
ctest

# Python tests (new test runner)
python3 tests/test_sdrplay.py

# Alternatively, run specific tests:
PYTHONPATH=. python3 -m unittest tests/test_sdrplay_parameters.py
```

## Troubleshooting

If you encounter the error "ImportError: cannot import name '_sdrplay' from partially initialized module 'sdrplay'", try these solutions:

1. Make sure you have built the library correctly:
   ```bash
   mkdir -p build && cd build
   cmake ..
   make
   ```

2. The Python module should be used as shown in the example:
   ```python
   import sdrplay
   # Create a device and work with it
   device = sdrplay.Device()
   ```

3. Run the example script to verify the installation:
   ```bash
   python3 example.py
   ```

## License

MIT