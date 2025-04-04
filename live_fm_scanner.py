#!/usr/bin/env python3
"""
Live FM Radio Station Scanner with Audio Playback
------------------------------------------------
Scans for FM radio stations and plays audio through soundcard.
Uses the SDRPlay API to capture real-time signals.
"""

import sys
import time
import signal as system_signal
import logging
import argparse
import numpy as np
import threading
import sounddevice as sd
import scipy.signal as scipy_signal
from collections import deque
from sdrplay import *

# UK FM stations with their frequencies
UK_FM_STATIONS = {
    87.5: "Local/Community",
    88.1: "BBC Radio 2",
    88.6: "BBC Radio Local",
    89.1: "BBC Radio 2",
    89.7: "BBC Radio 2",
    90.7: "BBC Radio 3",
    91.0: "BBC Radio 3",
    91.3: "BBC Radio 3",
    92.4: "BBC Radio 3",
    92.5: "BBC Radio Scotland",
    93.5: "BBC Radio 4",
    93.8: "BBC Radio 4",
    94.6: "BBC Radio 3",
    94.9: "BBC Radio 4",
    95.3: "BBC Radio Scotland",
    95.8: "BBC Radio 2",
    96.0: "Classic FM",
    96.3: "Classic FM",
    96.7: "BBC Radio 1",
    97.6: "BBC Radio 1",
    98.4: "BBC Radio 1",
    98.8: "BBC Radio 1",
    99.5: "Classic FM",
    100.4: "Classic FM",
    102.2: "BBC Radio 2",
    104.6: "Classic FM",
}

# Global variables
running = True
current_frequency = 100.0  # MHz
station_lock = threading.Lock()
audio_buffer = deque(maxlen=48000*2)  # 2 seconds of audio at 48kHz
signal_level = -100.0
found_stations = []

# Setup logging
logging.basicConfig(level=logging.INFO, 
                    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger('fm_scanner')

def signal_handler(sig, frame):
    """Handle Ctrl+C to gracefully exit"""
    global running
    logger.info("Stopping scanner...")
    running = False

# Setup signal handler for graceful exit
system_signal.signal(system_signal.SIGINT, signal_handler)

# Callback handler for stream data
class FMStreamCallback(StreamCallbackHandler):
    def __init__(self, sample_rate=2e6, audio_rate=48000):
        self.sample_rate = sample_rate
        self.audio_rate = audio_rate
        self.decimation = int(sample_rate / audio_rate)
        self.phase = 0
        self.last_sample = complex(0, 0)
        
        # Create low-pass filter for audio
        self.audio_filter = scipy_signal.firwin(101, 15000, fs=audio_rate)
        
    def handleStreamData(self, xi, xq, numSamples):
        global signal_level, audio_buffer
        
        try:
            # Convert to complex samples
            samples = xi[:numSamples].astype(np.float32) + 1j * xq[:numSamples].astype(np.float32)
            
            # Calculate signal power
            power = 10 * np.log10(np.mean(np.abs(samples)**2) + 1e-10)
            
            with station_lock:
                signal_level = power
            
            # FM demodulation
            # Compute phase difference between consecutive samples
            product = np.conj(samples[:-1]) * samples[1:]
            phase_diff = np.angle(product)
            
            # Scale for audio
            audio = phase_diff * (self.audio_rate / (2 * np.pi * 75e3))  # 75 kHz FM deviation
            
            # Decimate to audio rate
            audio_decimated = scipy_signal.resample_poly(audio, 1, self.decimation)
            
            # Filter audio
            audio_filtered = scipy_signal.lfilter(self.audio_filter, 1.0, audio_decimated)
            
            # Add to audio buffer
            with station_lock:
                audio_buffer.extend(audio_filtered)
        
        except Exception as e:
            logger.error(f"Error processing stream data: {e}")

# Callback handler for gain changes
class GainCallback(GainCallbackHandler):
    def handleGainChange(self, gRdB, lnaGRdB, currGain):
        logger.debug(f"Gain changed: GR={gRdB}dB, LNA GR={lnaGRdB}dB, Current={currGain:.2f}dB")

# Callback handler for power overloads
class PowerOverloadCallback(PowerOverloadCallbackHandler):
    def handlePowerOverload(self, isOverloaded):
        if isOverloaded:
            logger.warning("⚠️ POWER OVERLOAD DETECTED ⚠️")

def audio_callback(outdata, frames, time, status):
    """Callback for audio output stream"""
    global audio_buffer, station_lock
    
    if status:
        logger.warning(f"Audio status: {status}")
    
    with station_lock:
        # Get audio data from buffer
        if len(audio_buffer) >= frames:
            outdata[:, 0] = np.array(list(audio_buffer)[:frames])
            # Remove used samples
            for _ in range(frames):
                if audio_buffer:
                    audio_buffer.popleft()
        else:
            # Not enough data, fill with zeros
            outdata[:, 0] = np.zeros(frames)

def scan_fm_band(device, start_freq=87.5, end_freq=108.0, step=0.1, dwell_time=2.0, min_signal=-60):
    """
    Scan the FM band for active stations
    
    Parameters:
    - device: SDRPlay device object
    - start_freq: Start frequency in MHz
    - end_freq: End frequency in MHz
    - step: Frequency step in MHz
    - dwell_time: Time to dwell on each frequency in seconds
    - min_signal: Minimum signal level in dB to consider a station
    """
    global current_frequency, signal_level, found_stations
    
    print(f"Starting FM band scan from {start_freq} to {end_freq} MHz")
    print("=" * 70)
    print(f"{'Freq (MHz)':<12} {'Signal (dB)':<12} {'Station':<36} {'Status':<10}")
    print("-" * 70)
    
    # Scan the band
    freq = start_freq
    found_stations = []
    
    while freq <= end_freq and running:
        # Update current frequency (MHz -> Hz)
        with station_lock:
            current_frequency = freq
        
        # Tune the device
        device.setFrequency(freq * 1e6)
        
        # Wait for signal to stabilize and collect samples
        time.sleep(dwell_time)
        
        # Get signal level
        with station_lock:
            level = signal_level
        
        # Find closest known station
        closest_freq = min(UK_FM_STATIONS.keys(), key=lambda x: abs(x - freq))
        if abs(closest_freq - freq) <= 0.2:
            station_name = UK_FM_STATIONS[closest_freq]
        else:
            station_name = "Unknown Station"
        
        # Print status and determine if it's a valid station
        if level > min_signal:
            status = "ACTIVE"
            # Add to found stations
            found_stations.append((freq, level, station_name))
            print(f"{freq:<12.1f} {level:<12.1f} {station_name:<36} {status:<10}")
        else:
            status = "Weak"
            print(f"{freq:<12.1f} {level:<12.1f} {station_name:<36} {status:<10}")
        
        # Move to next frequency
        freq += step
    
    # Sort found stations by signal strength
    found_stations.sort(key=lambda x: x[1], reverse=True)
    
    print("=" * 70)
    print("Scan completed!")
    print(f"Found {len(found_stations)} stations with signal above {min_signal} dB")
    
    # Print top stations
    if found_stations:
        print("\nTop 10 strongest stations:")
        print(f"{'Freq (MHz)':<12} {'Signal (dB)':<12} {'Station':<36}")
        print("-" * 60)
        for i, (freq, level, name) in enumerate(found_stations[:10]):
            print(f"{freq:<12.1f} {level:<12.1f} {name:<36}")
    
    return found_stations

def play_station(device, frequency):
    """
    Tune to a specific station and play audio
    
    Parameters:
    - device: SDRPlay device object
    - frequency: Frequency in MHz
    """
    global current_frequency, audio_buffer, running
    
    # Clear audio buffer
    with station_lock:
        audio_buffer.clear()
        current_frequency = frequency
    
    # Tune to station
    device.setFrequency(frequency * 1e6)
    print(f"Tuned to {frequency:.1f} MHz. Playing audio...")
    
    # Find station name if known
    closest_freq = min(UK_FM_STATIONS.keys(), key=lambda x: abs(x - frequency))
    if abs(closest_freq - frequency) <= 0.2:
        station_name = UK_FM_STATIONS[closest_freq]
    else:
        station_name = "Unknown Station"
    
    print(f"Station: {station_name}")
    
    # Let audio play until user interrupts
    try:
        while running:
            with station_lock:
                level = signal_level
                
            # Update display periodically
            print(f"\rSignal: {level:.1f} dB | Press Ctrl+C to stop", end="")
            time.sleep(0.5)
    
    except KeyboardInterrupt:
        print("\nStopping playback...")
        running = False

def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='Live FM Radio Scanner with Audio')
    parser.add_argument('--frequency', type=float, default=100.0, 
                        help='Initial frequency in MHz (default: 100.0)')
    parser.add_argument('--scan', action='store_true',
                        help='Scan the FM band on startup')
    parser.add_argument('--min-freq', type=float, default=87.5,
                        help='Minimum frequency for scanning in MHz (default: 87.5)')
    parser.add_argument('--max-freq', type=float, default=108.0,
                        help='Maximum frequency for scanning in MHz (default: 108.0)')
    parser.add_argument('--step', type=float, default=0.1,
                        help='Frequency step for scanning in MHz (default: 0.1)')
    parser.add_argument('--dwell', type=float, default=2.0,
                        help='Dwell time on each frequency in seconds (default: 2.0)')
    parser.add_argument('--min-signal', type=float, default=-60,
                        help='Minimum signal level in dB to consider a station (default: -60)')
    args = parser.parse_args()
    
    # Create device
    device = Device()
    
    try:
        # Initialize device and start streaming
        devices = device.getAvailableDevices()
        if not devices:
            logger.error("No SDRPlay devices found")
            return 1
        
        logger.info(f"Found {len(devices)} device(s)")
        for i, dev in enumerate(devices):
            logger.info(f"Device {i}: {dev.serialNumber}, HW version: {dev.hwVer}")
        
        # Select the first available device
        if not device.selectDevice(devices[0]):
            logger.error("Failed to select device")
            return 1
        
        logger.info(f"Selected device: {devices[0].serialNumber}")
        
        # Setup basic parameters for FM reception
        device.setSampleRate(2e6)  # 2 MHz sample rate
        device.setFrequency(args.frequency * 1e6)  # Initial frequency
        
        # Create and register callbacks
        stream_cb = FMStreamCallback()
        gain_cb = GainCallback()
        power_cb = PowerOverloadCallback()
        
        device.registerStreamCallback(stream_cb)
        device.registerGainCallback(gain_cb)
        device.registerPowerOverloadCallback(power_cb)
        
        # Start streaming
        if not device.startStreaming():
            logger.error("Failed to start streaming")
            return 1
        
        logger.info("Streaming started successfully")
        
        # Start audio output
        with sd.OutputStream(channels=1, callback=audio_callback, 
                             samplerate=48000, blocksize=1024):
            logger.info("Audio output started")
            
            # Perform scan if requested
            stations = []
            if args.scan:
                stations = scan_fm_band(
                    device, 
                    start_freq=args.min_freq, 
                    end_freq=args.max_freq,
                    step=args.step,
                    dwell_time=args.dwell,
                    min_signal=args.min_signal
                )
            
            # Play strongest station from scan or the specified frequency
            if stations and args.scan:
                # Play the strongest station
                strongest_freq = stations[0][0]
                print(f"\nPlaying strongest station at {strongest_freq:.1f} MHz")
                play_station(device, strongest_freq)
            else:
                # Play specified frequency
                play_station(device, args.frequency)
        
    except Exception as e:
        logger.error(f"Error: {e}")
        return 1
    
    finally:
        # Stop streaming and clean up
        if device.isStreaming():
            device.stopStreaming()
            logger.info("Streaming stopped")
        
        device.releaseDevice()
        logger.info("Device released")
    
    return 0

if __name__ == "__main__":
    # Install required packages if not already installed
    try:
        import sounddevice
    except ImportError:
        print("Installing required packages...")
        import subprocess
        subprocess.check_call([sys.executable, "-m", "pip", "install", "sounddevice"])
        
    try:
        import scipy
    except ImportError:
        print("Installing scipy...")
        import subprocess
        subprocess.check_call([sys.executable, "-m", "pip", "install", "scipy"])
    
    sys.exit(main())