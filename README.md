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

- Fixed Python bindings for RSPdxR2 device support
- Added device detection and basic parameter control
- Updated SWIG interface for better Python integration
- Added debug logging to help diagnose device connection issues

## Usage

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

## Testing

```bash
# C++ tests
cd build
ctest

# Python tests
PYTHONPATH=. python3 -m unittest discover tests
```

## License

MIT