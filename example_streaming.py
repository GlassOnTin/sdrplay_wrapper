#!/usr/bin/env python3
"""
Example script showing how to use the SDRPlay wrapper streaming API
"""

import sys
import os
import time
import logging
import numpy as np

# Set up logging
logging.basicConfig(
    level=logging.DEBUG,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger("SDRPlayStreamingExample")

# Make sure we can find the sdrplay module
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

try:
    import sdrplay
    logger.info("Successfully imported SDRPlay module")
except ImportError as e:
    logger.error(f"Failed to import SDRPlay module: {e}")
    logger.error("Make sure the library has been built with 'cmake .. && make' in the build directory")
    sys.exit(1)

class StreamHandler(sdrplay.StreamCallbackHandler):
    def __init__(self):
        # Don't call parent constructor for SWIG abstract class
        self.samples_received = 0
        self.last_report = time.time()
        self.report_interval = 1.0  # seconds
    
    def handleStreamData(self, xi, xq, numSamples):
        # Convert to numpy arrays for easier processing
        try:
            i_data = np.array(xi[:numSamples])
            q_data = np.array(xq[:numSamples])
            
            # Record that we received samples
            self.samples_received += numSamples
            
            # Print stats periodically
            now = time.time()
            if now - self.last_report > self.report_interval:
                logger.info(f"Received {self.samples_received} samples in the last {now - self.last_report:.1f} seconds")
                # Reset counter
                self.samples_received = 0
                self.last_report = now
                
                # Calculate signal power
                power = np.mean(i_data**2 + q_data**2)
                logger.info(f"Signal power: {10*np.log10(power):.1f} dB")
        except Exception as e:
            logger.error(f"Error processing stream data: {e}")

class GainHandler(sdrplay.GainCallbackHandler):
    def handleGainChange(self, gRdB, lnaGRdB, currGain):
        logger.info(f"Gain changed - gRdB: {gRdB}, lnaGRdB: {lnaGRdB}, currGain: {currGain}")

class PowerHandler(sdrplay.PowerOverloadCallbackHandler):
    def handlePowerOverload(self, isOverloaded):
        if isOverloaded:
            logger.warning("POWER OVERLOAD DETECTED!")
        else:
            logger.info("Power overload cleared")

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
    
    try:
        # Set basic parameters
        device.setFrequency(100e6)  # 100 MHz
        device.setSampleRate(2e6)   # 2 MHz
        logger.info(f"Set frequency: {device.getFrequency()/1e6:.1f} MHz")
        logger.info(f"Set sample rate: {device.getSampleRate()/1e6:.1f} MHz")
        
        # Set up streaming callbacks
        stream_cb = StreamHandler()
        gain_cb = GainHandler()
        power_cb = PowerHandler()
        
        # Register callbacks - note that these will fail in the current implementation
        # due to SWIG binding issues, but are included for completeness
        logger.info("Setting up streaming (callback registration will fail - this is expected)")
        try:
            device.registerStreamCallback(stream_cb)
            device.registerGainCallback(gain_cb)
            device.registerPowerOverloadCallback(power_cb)
            logger.info("Callbacks registered successfully")
        except (AttributeError, TypeError) as e:
            logger.warning(f"Failed to register callbacks: {e}")
            logger.warning("This is expected in the current implementation")
        
        # Start streaming (will fail if callbacks are required)
        logger.info("Starting streaming...")
        if device.startStreaming():
            logger.info("Streaming started successfully")
            
            # Let it run for a few seconds
            seconds = 5
            logger.info(f"Streaming for {seconds} seconds...")
            time.sleep(seconds)
            
            # Stop streaming
            logger.info("Stopping streaming...")
            if device.stopStreaming():
                logger.info("Streaming stopped successfully")
            else:
                logger.error("Failed to stop streaming")
        else:
            logger.error("Failed to start streaming")
            logger.info("Note: Streaming may fail if callbacks aren't properly registered")
            
    except Exception as e:
        logger.error(f"Error during streaming: {e}")
    finally:
        # Cleanup
        device.releaseDevice()
        logger.info("Released device")

if __name__ == "__main__":
    main()