# SDRPlay Python Wrapper

This is a Python wrapper for the SDRPlay API, supporting RSP1A and RSPdx-R2 devices.

## Requirements

- SDRPlay API v3.0 or newer
- C++17 compatible compiler
- SWIG 4.0 or newer
- CMake 3.12 or newer
- Python 3.8 or newer
- NumPy (for streaming functionality)
- SciPy (for signal processing in examples)

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
- **NEW: Added streaming functionality with NumPy integration**
- **NEW: Added FM radio receiver example**

## Usage

### Basic Usage

```python
import sdrplay

# Initialize the device registry
sdrplay.initializeDeviceRegistry()

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
    if devices[0].hwVer == sdrplay.RSP1A_HWVER:
        rsp1a_params = device.getRsp1aParams()
        # Configure RSP1A-specific features
        rsp1a_params.setGainReduction(40)  # Set gain reduction to 40 dB
        rsp1a_params.setLNAState(0)        # Set LNA state
    
    elif devices[0].hwVer == sdrplay.RSPDXR2_HWVER:
        rspdxr2_params = device.getRspDxR2Params()
        # Configure RSPdxR2-specific features
        
    # When done
    device.releaseDevice()
```

### Streaming Example

```python
import numpy as np
import matplotlib.pyplot as plt
from sdrplay import *

# SampleCallbackHandler is a class that receives samples
class MySampleHandler(SampleCallbackHandler):
    def __init__(self):
        SampleCallbackHandler.__init__(self)
        self.samples = []
        
    def handleSamples(self, samples, count):
        # Convert samples to numpy array
        array_slice = np.array([complex(samples[i].real, samples[i].imag) 
                      for i in range(count)])
        self.samples.extend(array_slice)

# Initialize the device registry
initializeDeviceRegistry()

# Create and configure the device
device = Device()
devices = device.getAvailableDevices()
if devices:
    device.selectDevice(devices[0])
    device.setFrequency(100e6)  # 100 MHz
    device.setSampleRate(2e6)   # 2 MHz
    
    # Create the sample handler
    handler = MySampleHandler()
    
    # Set the callback and start streaming
    device.setPythonSampleCallback(handler)
    device.startStreaming()
    
    # Stream for 5 seconds
    import time
    time.sleep(5)
    
    # Stop streaming and release the device
    device.stopStreaming()
    device.setPythonSampleCallback(None)
    device.releaseDevice()
    
    # Process the collected samples
    if handler.samples:
        # Create a simple spectrum plot
        plt.figure(figsize=(10, 6))
        plt.psd(handler.samples, NFFT=1024, Fs=2e6, Fc=100e6)
        plt.title('Spectrum')
        plt.xlabel('Frequency (Hz)')
        plt.ylabel('Power Spectral Density (dB/Hz)')
        plt.savefig('spectrum.png')
        print("Spectrum saved to spectrum.png")
```

### Direct Read Example

```python
import numpy as np
from sdrplay import *

# Initialize the device registry
initializeDeviceRegistry()

# Create and configure the device
device = Device()
devices = device.getAvailableDevices()
if devices:
    device.selectDevice(devices[0])
    device.setFrequency(100e6)  # 100 MHz
    device.setSampleRate(2e6)   # 2 MHz
    
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

### FM Radio Receiver Example

A complete FM radio receiver example is included in `fm_radio_player.py`. It uses the streaming functionality to receive FM radio signals, demodulates them, and plays the audio through your computer's speakers.

To run the FM receiver:

```bash
python3 fm_radio_player.py --freq 100.3e6
```

Options:
- `--freq`: FM station frequency in Hz (default: 100.3 MHz)
- `--sample-rate`: Sample rate in Hz (default: 2 MHz)
- `--gain`: Gain reduction in dB (default: 40 dB)
- `--lna`: LNA state (default: 0)

During operation, you can:
- Enter `t 104.3` to tune to 104.3 MHz
- Enter `q` to quit

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

## Example Programs

- `streaming_example.py` - Demonstrates basic streaming functionality
- `fm_radio_player.py` - FM radio receiver with live audio output

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