#!/usr/bin/env python3
"""
Example script demonstrating how to use the SDRPlay streaming API
"""
import logging
import numpy as np
import time
import argparse
import sys
import signal
from sdrplay import sdrplay

# Set up logging
logging.basicConfig(level=logging.INFO,
                   format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# Define callback handlers
class StreamHandler(sdrplay.StreamCallbackHandler):
    def __init__(self):
        # Note: For abstract classes in SWIG, don't call the parent constructor
        self.buffer = []
        self.total_samples = 0
        self.sample_count = 0

    def handleStreamData(self, xi, xq, numSamples):
        # Convert to numpy arrays for easier processing
        i_data = np.array(xi[:numSamples])
        q_data = np.array(xq[:numSamples])
        
        # Simple statistics for demonstration
        i_mean = np.mean(i_data)
        q_mean = np.mean(q_data)
        i_std = np.std(i_data)
        q_std = np.std(q_data)
        
        self.total_samples += numSamples
        self.sample_count += 1
        
        if self.sample_count % 10 == 0:
            logger.info(f"Got {numSamples} samples (total: {self.total_samples})")
            logger.info(f"I mean: {i_mean:.2f}, std: {i_std:.2f}")
            logger.info(f"Q mean: {q_mean:.2f}, std: {q_std:.2f}")
            
            # Store a small sample for later analysis if needed
            if len(self.buffer) < 5:
                complex_samples = i_data[:10] + 1j * q_data[:10]
                self.buffer.append(complex_samples)

class GainHandler(sdrplay.GainCallbackHandler):
    def handleGainChange(self, gRdB, lnaGRdB, currGain):
        logger.info(f"Gain changed: gRdB={gRdB}, lnaGRdB={lnaGRdB}, currGain={currGain}")

class PowerHandler(sdrplay.PowerOverloadCallbackHandler):
    def handlePowerOverload(self, isOverloaded):
        if isOverloaded:
            logger.warning("POWER OVERLOAD DETECTED!")
        else:
            logger.info("Power overload corrected")

def signal_handler(sig, frame):
    global running
    logger.info("Stopping streaming...")
    running = False

def main():
    parser = argparse.ArgumentParser(description='SDRPlay Streaming Example')
    parser.add_argument('--freq', type=float, default=100.0,
                        help='Frequency in MHz (default: 100.0 MHz)')
    parser.add_argument('--samplerate', type=float, default=2.0,
                        help='Sample rate in Msps (default: 2.0 Msps)')
    parser.add_argument('--gain', type=int, default=40,
                        help='Gain reduction in dB (default: 40 dB)')
    parser.add_argument('--time', type=int, default=10,
                        help='Streaming time in seconds (default: 10)')
    
    args = parser.parse_args()
    
    # Convert to proper units
    freq_hz = args.freq * 1e6
    sample_rate_hz = args.samplerate * 1e6
    
    # Create device and select first available
    device = sdrplay.Device()
    
    logger.info("Getting available devices...")
    devices = device.getAvailableDevices()
    if not devices:
        logger.error("No devices found")
        return 1
    
    logger.info(f"Found {len(devices)} device(s)")
    for i, dev in enumerate(devices):
        logger.info(f"Device {i+1}: SN:{dev.serialNumber}, HwVer:{dev.hwVer}")
    
    # Select the first device
    device_info = devices[0]
    logger.info(f"Selecting device: {device_info.serialNumber}")
    if not device.selectDevice(device_info):
        logger.error("Failed to select device")
        return 1
    
    # Set up device parameters
    logger.info(f"Setting frequency to {freq_hz/1e6:.2f} MHz")
    device.setFrequency(freq_hz)
    
    logger.info(f"Setting sample rate to {sample_rate_hz/1e6:.2f} Msps")
    device.setSampleRate(sample_rate_hz)
    
    # Set up device-specific parameters
    if device_info.hwVer == sdrplay.RSP1A_HWVER:
        logger.info("Configuring RSP1A parameters")
        params = device.getRsp1aParams()
        if params:
            logger.info(f"Setting gain reduction to {args.gain} dB")
            params.setGainReduction(args.gain)
    elif device_info.hwVer == sdrplay.RSPDXR2_HWVER:
        logger.info("Configuring RSPdx R2 parameters")
        params = device.getRspDxR2Params()
        if params:
            logger.info(f"Setting gain reduction to {args.gain} dB")
            params.setGainReduction(args.gain)
    
    # Set up streaming callbacks
    stream_cb = StreamHandler()
    gain_cb = GainHandler()
    power_cb = PowerHandler()
    
    # Register callbacks
    device.registerStreamCallback(stream_cb)
    device.registerGainCallback(gain_cb)
    device.registerPowerOverloadCallback(power_cb)
    
    # Set up signal handler for graceful termination
    global running
    running = True
    signal.signal(signal.SIGINT, signal_handler)
    
    # Start streaming
    logger.info("Starting streaming...")
    if device.startStreaming():
        logger.info("Streaming started successfully")
        
        # Stream for specified time or until interrupted
        try:
            for i in range(args.time):
                if not running:
                    break
                logger.info(f"Streaming... {i+1}/{args.time} seconds")
                time.sleep(1)
        except KeyboardInterrupt:
            logger.info("User interrupted")
        
        # Stop streaming
        logger.info("Stopping streaming...")
        if device.stopStreaming():
            logger.info("Streaming stopped successfully")
        else:
            logger.error("Failed to stop streaming")
    else:
        logger.error("Failed to start streaming")
    
    # Release the device
    logger.info("Releasing device...")
    device.releaseDevice()
    
    if stream_cb.buffer:
        logger.info("Sample of received IQ data:")
        for i, samples in enumerate(stream_cb.buffer):
            logger.info(f"Buffer {i+1}: {samples}")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())