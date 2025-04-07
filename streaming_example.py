#!/usr/bin/env python3
"""
SDRplay Streaming Example

This example demonstrates how to use the streaming functionality of the SDRplay API wrapper.
It shows both callback-based streaming and direct sample reading, and demonstrates how to
process the samples with NumPy/SciPy.
"""

import time
import argparse
import numpy as np
import matplotlib.pyplot as plt
from sdrplay import *

class SampleHandler(SampleCallbackHandler):
    """
    Sample callback handler that processes samples as they arrive
    """
    def __init__(self, buffer_size=1024*1024):
        SampleCallbackHandler.__init__(self)
        self.samples = []
        self.max_samples = buffer_size
        self.sample_count = 0
        self.lock = False
        
    def handleSamples(self, samples, count):
        """Handle samples coming from the SDR device"""
        if self.lock:
            return
            
        # Convert samples to NumPy array - this is done internally
        # The samples parameter is already a numpy array of complex values
        if len(self.samples) < self.max_samples:
            # In a real application, you would process the samples here
            # For demonstration, we're just storing them
            array_slice = np.asarray([complex(samples[i].real, samples[i].imag) 
                          for i in range(count)])
            self.samples.extend(array_slice)
            self.sample_count += count
        
    def get_samples(self):
        """Get collected samples and clear the buffer"""
        self.lock = True
        samples = np.array(self.samples)
        self.samples = []
        self.lock = False
        return samples

class EventHandler(EventCallbackHandler):
    """
    Event callback handler that processes device events
    """
    def __init__(self):
        EventCallbackHandler.__init__(self)
        
    def handleEvent(self, eventType, params):
        """Handle events from the SDR device"""
        if eventType == EventType.GainChange:
            print(f"Gain Change Event: GR={params.gRdB} dB, LNA GR={params.lnaGRdB} dB, Current gain={params.currGain}")
        elif eventType == EventType.PowerOverload:
            print(f"Power Overload Event: {'Detected' if params.overloadDetected else 'Corrected'}")
        elif eventType == EventType.DeviceRemoved:
            print("Device Removed Event")
        else:
            print(f"Event: {eventType}")

def setup_device(args):
    """
    Set up the SDRplay device with the specified parameters
    
    Args:
        args: Command line arguments
        
    Returns:
        device: Configured SDRplay Device object
    """
    # Initialize the API
    print("Initializing SDRplay API...")
    initializeDeviceRegistry()
    
    # Create device object
    device = Device()
    
    # Get available devices
    devices = device.getAvailableDevices()
    if not devices:
        print("No SDRplay devices found")
        return None
        
    print(f"Found {len(devices)} device(s)")
    for i, dev_info in enumerate(devices):
        print(f"{i+1}: {dev_info.serialNumber} (hwVer={dev_info.hwVer})")
    
    # Select the first device
    if not device.selectDevice(devices[0]):
        print("Failed to select device")
        return None
    
    # Configure device parameters
    print(f"Setting frequency to {args.freq / 1e6:.3f} MHz")
    device.setFrequency(args.freq)
    
    print(f"Setting sample rate to {args.sample_rate / 1e6:.3f} Msps")
    device.setSampleRate(args.sample_rate)
    
    # Get device-specific parameters based on device type
    if devices[0].hwVer == RSP1A_HWVER:
        print("Device is RSP1A")
        params = device.getRsp1aParams()
        if params:
            # Set gain reduction (0-59 dB for RSP1A)
            print(f"Setting gain reduction to {args.gain} dB")
            params.setGainReduction(args.gain)
            params.setLNAState(args.lna)
    elif devices[0].hwVer == RSPDXR2_HWVER:
        print("Device is RSPdx/RSPdx R2")
        params = device.getRspDxR2Params()
        if params:
            # RSPdx-specific settings
            params.setHDRMode(args.hdr)
            
    return device

def callback_streaming_demo(device, duration=5):
    """
    Demonstrate callback-based streaming
    
    Args:
        device: SDRplay Device object
        duration: Duration to stream in seconds
    """
    # Create sample handler
    sample_handler = SampleHandler()
    
    # Create event handler
    event_handler = EventHandler()
    
    # Set callbacks
    device.setPythonSampleCallback(sample_handler)
    device.setPythonEventCallback(event_handler)
    
    # Start streaming
    print("Starting streaming (callback mode)...")
    if not device.startStreaming():
        print("Failed to start streaming")
        return
    
    try:
        # Stream for the specified duration
        print(f"Streaming for {duration} seconds...")
        for i in range(duration):
            time.sleep(1)
            print(f"Collected {sample_handler.sample_count} samples so far")
    finally:
        # Stop streaming
        print("Stopping streaming...")
        device.stopStreaming()
        
        # Clear callbacks to avoid memory leaks
        device.setPythonSampleCallback(None)
        device.setPythonEventCallback(None)
    
    # Process the collected samples
    samples = sample_handler.get_samples()
    print(f"Collected {len(samples)} samples in callback mode")
    
    # Basic signal processing example: compute and plot the spectrum
    if len(samples) > 0:
        plot_spectrum(samples, device.getSampleRate(), device.getFrequency())

def direct_read_demo(device, duration=5):
    """
    Demonstrate direct sample reading
    
    Args:
        device: SDRplay Device object
        duration: Duration to stream in seconds
    """
    # Start streaming without callbacks
    print("Starting streaming (direct read mode)...")
    if not device.startStreaming():
        print("Failed to start streaming")
        return
    
    try:
        # Create a NumPy array to store the samples
        all_samples = np.array([], dtype=np.complex64)
        
        # Stream for the specified duration
        end_time = time.time() + duration
        print(f"Reading samples for {duration} seconds...")
        
        while time.time() < end_time:
            # Check if samples are available
            available = device.samplesAvailable()
            
            if available > 0:
                # Read samples directly to a NumPy array
                samples = device.readSamplesToNumpy(available)
                
                # Append to our collection
                all_samples = np.append(all_samples, samples)
                
                print(f"Read {len(samples)} samples, total: {len(all_samples)}")
            
            # Sleep a little to avoid tight loop
            time.sleep(0.01)
    finally:
        # Stop streaming
        print("Stopping streaming...")
        device.stopStreaming()
    
    print(f"Collected {len(all_samples)} samples in direct read mode")
    
    # Basic signal processing example: compute and plot the spectrum
    if len(all_samples) > 0:
        plot_spectrum(all_samples, device.getSampleRate(), device.getFrequency())

def plot_spectrum(samples, sample_rate, center_freq):
    """
    Plot the spectrum of the samples
    
    Args:
        samples: NumPy array of complex samples
        sample_rate: Sample rate in Hz
        center_freq: Center frequency in Hz
    """
    # Compute FFT
    fft_size = min(8192, len(samples))
    
    # Use last samples to avoid any settling issues
    if len(samples) > fft_size:
        samples = samples[-fft_size:]
    
    # Apply window to reduce spectral leakage
    window = np.hanning(len(samples))
    windowed_samples = samples * window
    
    # Calculate the PSD
    fft = np.fft.fftshift(np.fft.fft(windowed_samples, fft_size))
    psd = 20 * np.log10(np.abs(fft))
    
    # Frequency axis
    freq = np.fft.fftshift(np.fft.fftfreq(fft_size, 1.0/sample_rate))
    freq_mhz = freq / 1e6 + center_freq / 1e6
    
    # Create the plot
    plt.figure(figsize=(10, 6))
    plt.plot(freq_mhz, psd)
    plt.grid(True)
    plt.xlabel('Frequency (MHz)')
    plt.ylabel('Power (dB)')
    plt.title(f'Spectrum at {center_freq / 1e6:.3f} MHz')
    
    # Add label for center frequency
    plt.axvline(x=center_freq / 1e6, color='r', linestyle='--')
    plt.text(center_freq / 1e6, np.max(psd) - 5, f"{center_freq / 1e6:.3f} MHz", 
             color='r', ha='center')
    
    plt.tight_layout()
    plt.savefig('spectrum.png')
    print("Spectrum saved to spectrum.png")
    plt.close()

def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='SDRplay Streaming Example')
    parser.add_argument('--freq', type=float, default=100e6, help='Frequency in Hz (default: 100 MHz)')
    parser.add_argument('--sample-rate', type=float, default=2e6, help='Sample rate in Hz (default: 2 MHz)')
    parser.add_argument('--gain', type=int, default=40, help='Gain reduction in dB (default: 40 dB)')
    parser.add_argument('--lna', type=int, default=0, help='LNA state (default: 0)')
    parser.add_argument('--duration', type=int, default=5, help='Duration to stream in seconds (default: 5)')
    parser.add_argument('--mode', choices=['callback', 'direct', 'both'], default='both', 
                        help='Streaming mode (default: both)')
    parser.add_argument('--hdr', action='store_true', help='Enable HDR mode (RSPdx only)')
    args = parser.parse_args()
    
    # Set up the device
    device = setup_device(args)
    if not device:
        return
    
    try:
        # Run the requested demo mode
        if args.mode in ['callback', 'both']:
            callback_streaming_demo(device, args.duration)
            
        if args.mode in ['direct', 'both']:
            direct_read_demo(device, args.duration)
    finally:
        # Release the device
        device.releaseDevice()

if __name__ == '__main__':
    main()