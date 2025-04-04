#!/usr/bin/env python3
"""
Example script showing how to use the SDRPlay wrapper
This demonstrates proper initialization and device usage
"""

import sys
import os
import logging

# Set up logging
logging.basicConfig(
    level=logging.DEBUG,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger("SDRPlayExample")

# Make sure we can find the sdrplay module
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

try:
    import sdrplay
    logger.info("Successfully imported SDRPlay module")
except ImportError as e:
    logger.error(f"Failed to import SDRPlay module: {e}")
    logger.error("Make sure the library has been built with 'cmake .. && make' in the build directory")
    sys.exit(1)

def main():
    # Create a device instance
    device = sdrplay.Device()
    logger.info("Created device object")
    
    # Get available devices
    devices = device.getAvailableDevices()
    logger.info(f"Found {len(devices)} devices")
    
    if len(devices) == 0:
        logger.warning("No SDRPlay devices found")
        return
    
    # Print device info
    for i, device_info in enumerate(devices):
        logger.info(f"Device {i}: {device_info.serialNumber}, HW Version: {device_info.hwVer}")
    
    # Select the first device
    device_info = devices[0]
    success = device.selectDevice(device_info)
    logger.info(f"Selected device: {'success' if success else 'failed'}")
    
    if not success:
        logger.error("Failed to select device")
        return
    
    # Set basic parameters
    device.setFrequency(100e6)  # 100 MHz
    device.setSampleRate(2e6)   # 2 MHz
    logger.info(f"Set frequency: {device.getFrequency()} Hz")
    logger.info(f"Set sample rate: {device.getSampleRate()} Hz")
    
    # Try to get device-specific parameters
    rsp1a_params = device.getRsp1aParams()
    rspdxr2_params = device.getRspDxR2Params()
    
    if rsp1a_params is not None:
        logger.info("Device is RSP1A")
        # Example RSP1A specific configuration
        # rsp1a_params.setGainReduction(40)
    
    if rspdxr2_params is not None:
        logger.info("Device is RSPdxR2")
        # Example RSPdxR2 specific configuration
        # rspdxr2_params.setHDRMode(True)
    
    # Cleanup
    device.releaseDevice()
    logger.info("Released device")

if __name__ == "__main__":
    main()