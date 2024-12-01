# SDRPlay Python Wrapper

A Python wrapper for the SDRPlay API using SWIG. This wrapper provides a clean Python interface to control SDRPlay software defined radio devices.

## Features

- Full Python interface to SDRPlay API
- Support for device discovery and configuration
- Stream, gain, and power overload callbacks
- Configurable timing parameters for different platforms
- Support for RSP devices (RSP1A, RSP2, RSPduo, RSPdx)

## Requirements

- SDRPlay API >= 3.15
- Python >= 3.8
- CMake >= 3.12
- SWIG >= 4.0
- C++17 compliant compiler
- SDRPlay service running (`systemctl status sdrplay`)

## Installation

### From Source

```bash
# Clone the repository
git clone https://github.com/glassontin/sdrplay_wrapper.git
cd sdrplay_wrapper

# Create and activate virtual environment (optional but recommended)
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install the package
pip install .
```

### System Dependencies

On Ubuntu/Debian:
```bash
sudo apt-get install cmake swig python3-dev
```

Ensure you have installed the SDRPlay API from [SDRPlay's website](https://www.sdrplay.com/downloads/).

## Usage

Basic example of device enumeration and configuration:

```python
import sdrplay

# Create device instance
device = sdrplay.Device()
device.open()

# List available devices
devices = device.getAvailableDevices()
if len(devices) > 0:
    # Select first device
    device.selectDevice(devices[0])

    # Configure device parameters
    params = device.getDeviceParams()
    params.setSampleRate(2e6)  # 2 MHz
    params.update()

    # Configure RX channel
    rx_params = device.getRxChannelParams()
    rx_params.setRfFrequency(100e6)  # 100 MHz
    rx_params.setBandwidth(600)      # 600 kHz
    rx_params.setGain(40, 0)         # 40 dB reduction
    rx_params.update()
```

Example with streaming callback:

```python
class StreamHandler(sdrplay.StreamCallbackHandler):
    def handleStreamData(self, xi, xq, numSamples):
        # Process IQ data
        print(f"Received {numSamples} samples")

# Start streaming
stream_handler = StreamHandler()
device.startStreamingWithHandlers(stream_handler)
```

## Testing

The package includes both Python and C++ tests:

```bash
# Run Python tests
python -m unittest tests/test_sdrplay.py -v

# Build and run C++ tests
cd tests
make
./test_sdrplay_api
```

## Platform-Specific Configuration

The wrapper includes configurable timing parameters that can be adjusted for different platforms:

```python
# Get current timing configuration
timing = device.getTiming()

# Adjust for slower systems (e.g., Raspberry Pi)
timing.openDelay = 2000    # 2 seconds
timing.selectDelay = 1000  # 1 second
device.setTiming(timing)
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

MIT, see [LICENSE](LICENSE)
