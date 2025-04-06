# SDRPlay Python Wrapper

This is a Python wrapper for the SDRPlay API, supporting RSP1A and RSPdx-R2 devices.

## Requirements

- SDRPlay API v3.0 or newer
- C++17 compatible compiler
- SWIG 4.0 or newer
- CMake 3.12 or newer
- Python 3.8 or newer

## Building

### Linux

```bash
mkdir -p build && cd build
cmake ..
make
```

### Windows

Using Visual Studio:

```batch
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -D SDRPLAY_API_INCLUDE_DIR=C:/path/to/SDRplay/API/inc -D SDRPLAY_API_LIBRARY=C:/path/to/SDRplay/API/x64 -D SWIG_EXECUTABLE=C:/path/to/swigwin/swig.exe ..
cmake --build . --config Release
```

Notes:
- Use forward slashes in paths (e.g., `C:/path/to/SDRplay`)
- For the `SDRPLAY_API_LIBRARY` parameter, specify the directory containing the .lib files, not the .lib file itself
- Make sure to match the architecture (x64 or x86) with your Visual Studio configuration

## Recent Updates

- Added device detection and connection support
- Implemented frequency tuning and parameter control
- Added support for RSP1A and RSPdxR2 devices
- Added device-specific parameter configuration
- Updated SWIG interface for Python integration
- Fixed device registry and implemented clearFactories method
- Added debug logging for troubleshooting
- Removed problematic streaming implementation to prepare for redesign

**IMPORTANT**: The streaming functionality has been temporarily removed and is NOT currently implemented. 
A new streaming implementation is planned for a future release. The streaming code examples shown below
are for reference only and will not work with the current version.

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

**NOTE: This section describes the planned streaming API that will be implemented in a future release.**

The future SDRPlay wrapper will provide a callback-based streaming API to receive IQ samples and handle events from the device. The planned callback interfaces are:

- **StreamCallbackHandler**: Will receive IQ sample data from the device
- **GainCallbackHandler**: Will be notified when gain changes occur
- **PowerOverloadCallbackHandler**: Will be notified when power overload conditions occur

A simple usage example will look like this (NOT YET IMPLEMENTED):

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

### Future Callback Bridge Implementation

The future implementation may include a more robust callback bridge to overcome limitations in the SWIG director callbacks:

```python
# NOTE: This code is for reference only and is NOT yet implemented

from sdrplay.callback_bridge import CallbackBridge  # Not yet available

# Create and initialize the bridge
bridge = CallbackBridge()
if not bridge.initialize():
    print("Failed to initialize!")
    exit(1)

# Register a callback function
def process_iq(iq_data, num_samples):
    # Process IQ data (complex numpy array)
    signal_level = np.mean(np.abs(iq_data))
    print(f"Received {num_samples} samples, signal level: {signal_level:.2f}")

bridge.register_callback("my_processor", process_iq)

# Configure device
bridge.set_frequency(100.5e6)  # 100.5 MHz
bridge.set_sample_rate(2.048e6)  # 2.048 MSPS
bridge.set_gain_reduction(40)  # 40 dB gain reduction

# Start streaming
bridge.start_streaming()

# Let it run for a while
time.sleep(10)

# Clean up
bridge.stop_streaming()
```

### Planned Example Applications

Future releases will include example applications demonstrating the callback bridge functionality:

1. **fm_radio.py**: A simple FM radio that tunes to a specified frequency and plays audio
2. **fm_scanner.py**: An FM band scanner that finds active stations

Usage will be something like:

```bash
# Play FM radio at 100.5 MHz
./fm_radio.py 100.5

# Scan the FM band from 88 to 108 MHz
./fm_scanner.py 88 108
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

### Common Issues

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

### Windows-Specific Issues

1. **Cannot find sdrplay_api.h**:
   - Make sure SDRPLAY_API_INCLUDE_DIR points to the directory containing sdrplay_api.h
   - Use forward slashes in paths: `C:/Program Files/SDRplay/API/inc`

2. **CMake linking warnings**:
   - When specifying SDRPLAY_API_LIBRARY, point to the directory containing the .lib file, not the .lib file itself
   - Example: `-D SDRPLAY_API_LIBRARY=C:/Program Files/SDRplay/API/x64`

3. **Python module import issues**:
   - Check that the .pyd file (Windows Python extension) was built correctly in the sdrplay/ directory
   - Verify that the site-packages directory contains both the .dist-info directory and the actual module files

4. **DLL not found errors**:
   - Ensure the SDRplay API DLL is in your system PATH or in the same directory as your Python application
   - You may need to copy sdrplay_api.dll from the SDRplay API directory to your application directory

## License

MIT