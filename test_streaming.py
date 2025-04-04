#!/usr/bin/env python3
"""
Simple script to test the streaming functionality without using the full unittest framework
"""
import time
from sdrplay import sdrplay

# Create a simple handler for testing
class StreamHandler(sdrplay.StreamCallbackHandler):
    def __init__(self):
        # For abstract classes in SWIG, don't call the parent constructor
        pass
        
    def handleStreamData(self, xi, xq, numSamples):
        print(f"Got {numSamples} samples")

class GainHandler(sdrplay.GainCallbackHandler):
    def __init__(self):
        # For abstract classes in SWIG, don't call the parent constructor
        pass
        
    def handleGainChange(self, gRdB, lnaGRdB, currGain):
        print(f"Gain changed: {gRdB}, {lnaGRdB}, {currGain}")

class PowerHandler(sdrplay.PowerOverloadCallbackHandler):
    def __init__(self):
        # For abstract classes in SWIG, don't call the parent constructor
        pass
        
    def handlePowerOverload(self, isOverloaded):
        print(f"Power overload: {isOverloaded}")

print("Creating device...")
device = sdrplay.Device()

print("Getting available devices...")
devices = device.getAvailableDevices()
print(f"Found {len(devices)} devices")

if len(devices) == 0:
    print("No devices found, exiting")
    exit(1)

print(f"Selecting device {devices[0].serialNumber}")
device.selectDevice(devices[0])

print("Setting frequency and sample rate...")
device.setFrequency(100e6)
device.setSampleRate(2e6)

print("Creating and registering callbacks...")
stream_handler = StreamHandler()
gain_handler = GainHandler()
power_handler = PowerHandler()

device.registerStreamCallback(stream_handler)
device.registerGainCallback(gain_handler)
device.registerPowerOverloadCallback(power_handler)

print("Starting streaming...")
result = device.startStreaming()
print(f"Streaming started: {result}")
print(f"Is streaming: {device.isStreaming()}")

print("Streaming for 5 seconds...")
time.sleep(5)

print("Stopping streaming...")
device.stopStreaming()
print(f"Is streaming: {device.isStreaming()}")

print("Releasing device...")
device.releaseDevice()
print("Done")