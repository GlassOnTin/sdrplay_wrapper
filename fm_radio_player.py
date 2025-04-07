#!/usr/bin/env python3
"""
SDRplay FM Radio Player Example

This example demonstrates a simple FM radio receiver using the SDRplay wrapper's
streaming functionality and the SciPy library for signal processing.
"""

import sys
import time
import argparse
import numpy as np
import scipy.signal as signal
import sounddevice as sd
import threading
from sdrplay import *

class FmDemodulator:
    """
    FM demodulator class for processing FM radio signals
    """
    def __init__(self, sample_rate, audio_rate=48000, stereo=False):
        self.sample_rate = sample_rate
        self.audio_rate = audio_rate
        self.stereo = stereo
        
        # Calculate downsampling parameters
        self.decimation = int(sample_rate / audio_rate)
        self.actual_audio_rate = sample_rate / self.decimation
        
        # Set up filters
        self.lowpass_filter = self._create_lowpass_filter(100e3, sample_rate)
        
        # Deemphasis time constant (75us in US, 50us in Europe)
        self.deemphasis_tc = 75e-6
        
        # Phase difference between consecutive samples for FM demodulation
        self.prev_phase = 0
        
        # Create audio buffer
        self.audio_buffer = np.zeros(16384, dtype=np.float32)
        self.buffer_pos = 0
        self.buffer_ready = threading.Event()
        
        # Audio stream
        self.audio_stream = None
        
    def _create_lowpass_filter(self, cutoff, sample_rate):
        """Create a low-pass filter with the given cutoff frequency"""
        taps = 64
        return signal.firwin(taps, cutoff, fs=sample_rate)
        
    def start_audio(self):
        """Start the audio output stream"""
        self.audio_stream = sd.OutputStream(
            samplerate=self.actual_audio_rate,
            channels=1,
            dtype='float32',
            callback=self._audio_callback
        )
        self.audio_stream.start()
    
    def stop_audio(self):
        """Stop the audio output stream"""
        if self.audio_stream:
            self.audio_stream.stop()
            self.audio_stream.close()
            self.audio_stream = None
            
    def _audio_callback(self, outdata, frames, time, status):
        """Callback function for the audio output stream"""
        if status:
            print(f"Audio callback status: {status}")
            
        # Wait for buffer to be ready if it's not
        if not self.buffer_ready.is_set():
            self.buffer_ready.wait(timeout=0.1)
            if not self.buffer_ready.is_set():
                outdata.fill(0)
                return
                
        # Copy data from our buffer to the output
        if frames <= len(self.audio_buffer):
            outdata[:] = self.audio_buffer[:frames].reshape(-1, 1)
            # Shift remaining data to the beginning of the buffer
            remaining = len(self.audio_buffer) - frames
            self.audio_buffer[:remaining] = self.audio_buffer[frames:frames+remaining]
            self.buffer_pos = remaining
            
            # If buffer is nearly empty, clear the ready flag
            if self.buffer_pos < frames:
                self.buffer_ready.clear()
        else:
            # Not enough data, fill with zeros
            outdata[:self.buffer_pos] = self.audio_buffer[:self.buffer_pos].reshape(-1, 1)
            outdata[self.buffer_pos:] = 0
            self.buffer_pos = 0
            self.buffer_ready.clear()
            
    def process_samples(self, samples):
        """
        Process IQ samples from SDR and demodulate FM
        
        Args:
            samples: NumPy array of complex samples
            
        Returns:
            Demodulated and filtered audio samples
        """
        # Apply low-pass filter to reduce noise
        filtered = signal.lfilter(self.lowpass_filter, 1.0, samples)
        
        # FM demodulation (phase difference)
        product = filtered[1:] * np.conj(filtered[:-1])
        angles = np.angle(product)
        
        # Account for previous buffer
        if self.prev_phase != 0:
            first_angle = np.angle(filtered[0] * np.conj(self.prev_phase))
            angles = np.insert(angles, 0, first_angle)
        self.prev_phase = filtered[-1]
        
        # Scale demodulated signal
        demodulated = angles * self.sample_rate / (2 * np.pi)
        
        # Deemphasis filter (simple IIR)
        alpha = 1.0 / (self.deemphasis_tc * self.sample_rate + 1.0)
        deemphasized = signal.lfilter([alpha], [1, -(1 - alpha)], demodulated)
        
        # Decimate to audio sample rate
        audio = signal.decimate(deemphasized, self.decimation)
        
        # Normalize audio to -1.0 to 1.0 range
        if np.max(np.abs(audio)) > 0:
            audio = audio / np.max(np.abs(audio)) * 0.7
            
        # Add to audio buffer
        remaining_space = len(self.audio_buffer) - self.buffer_pos
        if len(audio) <= remaining_space:
            self.audio_buffer[self.buffer_pos:self.buffer_pos + len(audio)] = audio
            self.buffer_pos += len(audio)
        else:
            # Buffer full, overwrite from the beginning
            self.audio_buffer[:len(audio)] = audio
            self.buffer_pos = len(audio)
            
        self.buffer_ready.set()
        
        return audio

class FmRadioReceiver(SampleCallbackHandler):
    """
    FM radio receiver using SDRplay API
    """
    def __init__(self, args):
        SampleCallbackHandler.__init__(self)
        self.args = args
        self.device = None
        self.demodulator = FmDemodulator(
            sample_rate=args.sample_rate,
            audio_rate=args.audio_rate,
            stereo=args.stereo
        )
        self.running = False
        
    def setup(self):
        """Set up the SDR device and start streaming"""
        # Initialize the API
        print("Initializing SDRplay API...")
        initializeDeviceRegistry()
        
        # Create device object
        self.device = Device()
        
        # Get available devices
        devices = self.device.getAvailableDevices()
        if not devices:
            print("No SDRplay devices found")
            return False
            
        print(f"Found {len(devices)} device(s)")
        for i, dev_info in enumerate(devices):
            print(f"{i+1}: {dev_info.serialNumber} (hwVer={dev_info.hwVer})")
        
        # Select the first device
        if not self.device.selectDevice(devices[0]):
            print("Failed to select device")
            return False
        
        # Configure device parameters
        print(f"Setting frequency to {self.args.freq / 1e6:.3f} MHz")
        self.device.setFrequency(self.args.freq)
        
        print(f"Setting sample rate to {self.args.sample_rate / 1e6:.3f} Msps")
        self.device.setSampleRate(self.args.sample_rate)
        
        # Get device-specific parameters based on device type
        if devices[0].hwVer == RSP1A_HWVER:
            print("Device is RSP1A")
            params = self.device.getRsp1aParams()
            if params:
                # Set gain reduction (0-59 dB for RSP1A)
                print(f"Setting gain reduction to {self.args.gain} dB")
                params.setGainReduction(self.args.gain)
                params.setLNAState(self.args.lna)
        elif devices[0].hwVer == RSPDXR2_HWVER:
            print("Device is RSPdx/RSPdx R2")
            params = self.device.getRspDxR2Params()
            if params:
                # RSPdx-specific settings
                pass
        
        # Set up the callback
        self.device.setPythonSampleCallback(self)
        
        # Start the audio
        self.demodulator.start_audio()
        
        # Start streaming
        print("Starting streaming...")
        if not self.device.startStreaming():
            print("Failed to start streaming")
            return False
            
        self.running = True
        return True
        
    def handleSamples(self, samples, count):
        """Handle samples from the SDR device"""
        if not self.running:
            return
            
        # Create a NumPy array from the samples
        # (This is necessary for the Python callback, the direct read methods
        # already return NumPy arrays)
        np_samples = np.array([complex(samples[i].real, samples[i].imag) 
                      for i in range(count)], dtype=np.complex64)
        
        # Process the samples through our FM demodulator
        self.demodulator.process_samples(np_samples)
        
    def tune(self, freq):
        """Tune to a new frequency"""
        if self.device:
            print(f"Tuning to {freq / 1e6:.3f} MHz")
            self.device.setFrequency(freq)
            
    def stop(self):
        """Stop streaming and release resources"""
        self.running = False
        
        # Stop audio
        self.demodulator.stop_audio()
        
        # Stop streaming
        if self.device:
            print("Stopping streaming...")
            self.device.stopStreaming()
            
            # Clear callbacks
            self.device.setPythonSampleCallback(None)
            
            # Release the device
            self.device.releaseDevice()
            self.device = None

def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='SDRplay FM Radio Example')
    parser.add_argument('--freq', type=float, default=100.3e6, help='Frequency in Hz (default: 100.3 MHz)')
    parser.add_argument('--sample-rate', type=float, default=2e6, help='Sample rate in Hz (default: 2 MHz)')
    parser.add_argument('--audio-rate', type=int, default=48000, help='Audio sample rate in Hz (default: 48 kHz)')
    parser.add_argument('--gain', type=int, default=40, help='Gain reduction in dB (default: 40 dB)')
    parser.add_argument('--lna', type=int, default=0, help='LNA state (default: 0)')
    parser.add_argument('--stereo', action='store_true', help='Enable stereo decoding (if available)')
    args = parser.parse_args()
    
    # Create and set up the FM receiver
    receiver = FmRadioReceiver(args)
    if not receiver.setup():
        return
    
    print("FM Radio Receiver running. Press Ctrl+C to exit.")
    try:
        while True:
            # Simple command interface
            cmd = input("Enter command (q=quit, t=tune, g=gain): ")
            if cmd.lower() == 'q':
                break
            elif cmd.lower().startswith('t'):
                try:
                    # Parse frequency in MHz
                    parts = cmd.split()
                    if len(parts) > 1:
                        freq = float(parts[1]) * 1e6
                        receiver.tune(freq)
                except ValueError:
                    print("Invalid frequency format. Use 't 100.3' for 100.3 MHz")
            elif cmd.lower().startswith('g'):
                try:
                    # Parse gain in dB
                    parts = cmd.split()
                    if len(parts) > 1:
                        gain = int(parts[1])
                        # This would require additional code to change gain during runtime
                        print(f"Gain change to {gain} dB not implemented yet")
                except ValueError:
                    print("Invalid gain format. Use 'g 40' for 40 dB")
    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        # Clean up
        receiver.stop()

if __name__ == '__main__':
    main()