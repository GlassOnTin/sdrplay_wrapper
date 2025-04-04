#!/usr/bin/env python3
import sdrplay

# Initialize the device registry
print("Initializing device registry...")
sdrplay.initializeDeviceRegistry()

# Create a device
print("Creating device...")
device = sdrplay.Device()

# Get available devices
print("Checking for available devices...")
devices = device.getAvailableDevices()
print(f"Found {len(devices)} devices")

# Display device info
for i, dev in enumerate(devices):
    print(f"Device {i+1}:")
    print(f"  Serial Number: {dev.serialNumber}")
    print(f"  Hardware Version: {dev.hwVer}")
    
# If we found a device, try to select it
if len(devices) > 0:
    print("\nSelecting device...")
    device.selectDevice(devices[0])
    
    # Try to set frequency and sample rate
    freq = 100e6  # 100 MHz
    rate = 2e6    # 2 MHz
    
    print(f"Setting frequency to {freq/1e6} MHz")
    device.setFrequency(freq)
    
    print(f"Setting sample rate to {rate/1e6} MHz")
    device.setSampleRate(rate)
    
    # Get current settings
    print(f"Current frequency: {device.getFrequency()/1e6} MHz")
    print(f"Current sample rate: {device.getSampleRate()/1e6} MHz")
    
    # Try to get device parameters
    print("\nTrying to get device parameters...")
    
    rsp1a_params = device.getRsp1aParams()
    if rsp1a_params:
        print("This is an RSP1A device")
    
    rspdxr2_params = device.getRspDxR2Params()
    if rspdxr2_params:
        print("This is an RSPdxR2 device")
        
    # Release the device
    device.releaseDevice()
else:
    print("No devices found to test with")
    
print("Test complete")