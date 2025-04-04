#!/usr/bin/env python3
"""
Example showing how to use the SDRPlay streaming API.
This example demonstrates:
- Setting up device parameters
- Registering callbacks for streaming data, gain changes, and power overloads
- Starting and stopping streaming
- Processing real-time data
"""

import sys
import time
import signal
import logging
import argparse
import numpy as np
from sdrplay import *

# Setup logging
logging.basicConfig(level=logging.INFO, 
                    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger('sdrplay_example')

# Flag to control the streaming loop
running = True

def signal_handler(sig, frame):
    """Handle Ctrl+C to gracefully exit"""
    global running
    logger.info("Stopping streaming...")
    running = False

# Setup signal handler for graceful exit
signal.signal(signal.SIGINT, signal_handler)

# Callback handler for stream data
class StreamCallback(StreamCallbackHandler):
    def __init__(self):
        super(StreamCallbackHandler, self).__init__()
        self.samples_received = 0
        
    def handleStreamData(self, xi, xq, numSamples):
        # Convert raw samples to numpy arrays for easier processing
        i_data = np.frombuffer(xi, dtype=np.int16, count=numSamples)
        q_data = np.frombuffer(xq, dtype=np.int16, count=numSamples)
        
        # Calculate power
        power = np.mean(i_data**2 + q_data**2)
        
        self.samples_received += numSamples
        if self.samples_received % 1000000 == 0:
            logger.info(f"Received {self.samples_received} samples, current power: {power:.2f}")

# Callback handler for gain changes
class GainCallback(GainCallbackHandler):
    def handleGainChange(self, gRdB, lnaGRdB, currGain):
        logger.info(f"Gain changed: GR={gRdB}dB, LNA GR={lnaGRdB}dB, Current={currGain:.2f}dB")

# Callback handler for power overloads
class PowerOverloadCallback(PowerOverloadCallbackHandler):
    def handlePowerOverload(self, isOverloaded):
        if isOverloaded:
            logger.warning("⚠️ POWER OVERLOAD DETECTED ⚠️")
        else:
            logger.info("Power overload condition cleared")

def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='SDRPlay Streaming Example')
    parser.add_argument('--frequency', type=float, default=98.8e6, 
                        help='Frequency in Hz (default: 98.8 MHz)')
    parser.add_argument('--sample-rate', type=float, default=2e6, 
                        help='Sample rate in Hz (default: 2 MHz)')
    parser.add_argument('--gain', type=int, default=40, 
                        help='Gain reduction in dB (default: 40, range: 20-59)')
    parser.add_argument('--time', type=int, default=10, 
                        help='Time to stream in seconds (default: 10)')
    args = parser.parse_args()
    
    # Initialize the device registry
    initializeDeviceRegistry()
    
    # Create device registry and check for devices
    registry = DeviceRegistry()
    devices = registry.getAvailableDevices()
    
    if not devices:
        logger.error("No SDRPlay devices found")
        return 1
    
    logger.info(f"Found {len(devices)} device(s)")
    for i, dev in enumerate(devices):
        logger.info(f"Device {i}: {dev.serialNumber}, HW version: {dev.hwVer}")
    
    # Create device and select the first available one
    device = registry.createDevice(devices[0].hwVer)
    if not device.selectDevice(devices[0]):
        logger.error("Failed to select device")
        return 1
    
    logger.info(f"Selected device: {devices[0].serialNumber}")
    
    # Setup basic parameters
    device.setFrequency(args.frequency)
    device.setSampleRate(args.sample_rate)
    device.setGainReduction(args.gain)
    
    logger.info(f"Configured device: Freq={args.frequency/1e6:.3f} MHz, "
                f"Sample Rate={args.sample_rate/1e6:.1f} MHz, "
                f"Gain Reduction={args.gain} dB")
    
    # Create callback handlers
    stream_cb = StreamCallback()
    gain_cb = GainCallback()
    power_cb = PowerOverloadCallback()
    
    # Register callbacks
    device.registerStreamCallback(stream_cb)
    device.registerGainCallback(gain_cb)
    device.registerPowerOverloadCallback(power_cb)
    
    # Start streaming
    if device.startStreaming():
        logger.info("Streaming started successfully")
        
        # Stream for specified time or until interrupted
        try:
            for i in range(args.time):
                if not running:
                    break
                logger.info(f"Streaming... {i+1}/{args.time} seconds")
                time.sleep(1)
        except Exception as e:
            logger.error(f"Error during streaming: {e}")
        finally:
            # Stop streaming
            if device.isStreaming():
                device.stopStreaming()
                logger.info("Streaming stopped")
    else:
        logger.error("Failed to start streaming")
        return 1
    
    logger.info(f"Received a total of {stream_cb.samples_received} samples")
    return 0

if __name__ == "__main__":
    sys.exit(main())