#!/usr/bin/env python3
"""
Simple script to test basic device functionality
"""
import time
from sdrplay import sdrplay

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
print(f"Frequency: {device.getFrequency()}")
print(f"Sample rate: {device.getSampleRate()}")

print("Releasing device...")
device.releaseDevice()
print("Done")