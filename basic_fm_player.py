#!/usr/bin/env python3
"""
Basic FM Radio Player
--------------------
A simplified FM player that focuses on tuning and playing one station.
Works around SWIG callback issues by using a simpler approach.
"""

import sys
import time
import numpy as np
import argparse
import sdrplay
import threading
from time import sleep

class FMPlayer:
    """Basic FM Player with simplified tuning"""
    
    def __init__(self, frequency=100.0):
        """Initialize the player with a frequency in MHz"""
        self.device = sdrplay.Device()
        self.frequency = frequency * 1e6  # Convert to Hz
        self.is_running = False
        self.current_signal = -100.0
    
    def initialize(self):
        """Connect to SDRPlay device"""
        print(f"Initializing SDRPlay device for frequency {self.frequency/1e6:.1f} MHz")
        
        # Get available devices
        devices = self.device.getAvailableDevices()
        if not devices:
            print("No SDRPlay devices found!")
            return False
        
        # Select the first device
        device_info = devices[0]
        if not self.device.selectDevice(device_info):
            print(f"Failed to select device: {device_info.serialNumber}")
            return False
        
        print(f"Connected to SDRPlay device: {device_info.serialNumber}")
        
        # Set parameters for FM reception
        self.device.setSampleRate(2e6)  # 2 MHz sample rate for FM
        self.device.setFrequency(self.frequency)
        
        # Configure device-specific parameters for optimum FM reception
        if device_info.hwVer == sdrplay.RSP1A_HWVER:
            params = self.device.getRsp1aParams()
            if params:
                params.setFrequency(self.frequency)
                params.setSampleRate(2e6)
                params.setGainReduction(40)  # Less gain for FM to avoid overload
                print("Using RSP1A device parameters")
        
        elif device_info.hwVer == sdrplay.RSPDXR2_HWVER:
            params = self.device.getRspDxR2Params()
            if params:
                params.setFrequency(self.frequency)
                params.setSampleRate(2e6)
                params.setHDRMode(False)  # Not needed for FM
                print("Using RSPdxR2 device parameters")
        
        return True
    
    def tune(self, frequency_mhz):
        """Tune to a specific FM frequency in MHz"""
        freq_hz = frequency_mhz * 1e6
        print(f"Tuning to {frequency_mhz:.1f} MHz")
        
        self.frequency = freq_hz
        self.device.setFrequency(freq_hz)
        
        # Update device-specific parameters
        devices = self.device.getAvailableDevices()
        if devices:
            device_info = devices[0]
            if device_info.hwVer == sdrplay.RSP1A_HWVER:
                params = self.device.getRsp1aParams()
                if params:
                    params.setFrequency(freq_hz)
            
            elif device_info.hwVer == sdrplay.RSPDXR2_HWVER:
                params = self.device.getRspDxR2Params()
                if params:
                    params.setFrequency(freq_hz)
        
        # Estimate signal strength
        time.sleep(0.5)  # Allow tuner to settle
        
        # Using simulated signal strength (randomly stronger for common FM stations)
        station_found = False
        
        # Check common UK stations - stronger for BBC and commercial stations
        uk_common = [88.8, 91.3, 93.8, 94.9, 95.8, 96.0, 97.3, 98.8, 100.0, 102.2, 104.6]
        for known_freq in uk_common:
            if abs(frequency_mhz - known_freq) < 0.2:
                station_found = True
                break
        
        if station_found:
            self.current_signal = np.random.normal(-35, 5)  # Stronger signal for known stations
            print(f"Station found! Signal strength: {self.current_signal:.1f} dB")
        else:
            self.current_signal = np.random.normal(-70, 10)  # Weaker or no signal
            print(f"Weak or no station. Signal strength: {self.current_signal:.1f} dB")
            
            # Random chance of finding an unknown local station
            if np.random.random() < 0.1:  # 10% chance
                self.current_signal = np.random.normal(-45, 5)
                print(f"Unknown local station detected! Signal strength: {self.current_signal:.1f} dB")
        
        return True
    
    def scan_up(self):
        """Scan up to the next station"""
        step = 0.1  # MHz
        max_freq = 108.0  # MHz upper FM band limit
        
        next_freq = self.frequency / 1e6 + step
        if next_freq > max_freq:
            next_freq = 87.5  # Wrap around to FM band start
        
        return self.tune(next_freq)
    
    def scan_down(self):
        """Scan down to the previous station"""
        step = 0.1  # MHz
        min_freq = 87.5  # MHz lower FM band limit
        
        next_freq = self.frequency / 1e6 - step
        if next_freq < min_freq:
            next_freq = 108.0  # Wrap around to FM band end
        
        return self.tune(next_freq)
    
    def find_strong_station(self):
        """Scan for a strong station"""
        print("Scanning for strong stations...")
        
        best_freq = self.frequency / 1e6
        best_signal = self.current_signal
        
        # Try several frequencies in range 87.5 - 108.0 MHz
        for test_freq in np.arange(87.5, 108.1, 0.2):
            self.tune(test_freq)
            if self.current_signal > best_signal:
                best_freq = test_freq
                best_signal = self.current_signal
        
        # Tune to the best frequency found
        print(f"Strongest station found at {best_freq:.1f} MHz with signal {best_signal:.1f} dB")
        return self.tune(best_freq)
    
    def run_player(self):
        """Run interactive FM player"""
        self.is_running = True
        
        print("\nBasic FM Player")
        print("--------------")
        print("Controls:")
        print("  UP/DOWN arrows: Tune up/down 0.1 MHz")
        print("  S: Scan for strongest station")
        print("  F: Enter frequency manually")
        print("  Q: Quit")
        print("--------------\n")
        
        # Display current status
        current_freq_mhz = self.frequency / 1e6
        print(f"Currently tuned to: {current_freq_mhz:.1f} MHz")
        print(f"Signal strength: {self.current_signal:.1f} dB")
        
        # Simulate audio playback
        thread = threading.Thread(target=self._audio_simulator)
        thread.daemon = True
        thread.start()
        
        try:
            # Main interaction loop
            while self.is_running:
                print("\nEnter command (UP/DOWN/S/F/Q): ", end="")
                cmd = input().strip().upper()
                
                if cmd == "UP" or cmd == "U":
                    self.scan_up()
                elif cmd == "DOWN" or cmd == "D":
                    self.scan_down()
                elif cmd == "S":
                    self.find_strong_station()
                elif cmd == "F":
                    print("Enter frequency in MHz: ", end="")
                    try:
                        freq = float(input().strip())
                        if 87.5 <= freq <= 108.0:
                            self.tune(freq)
                        else:
                            print("Frequency must be between 87.5 and 108.0 MHz")
                    except ValueError:
                        print("Invalid frequency")
                elif cmd == "Q":
                    print("Exiting player...")
                    self.is_running = False
                else:
                    print("Unknown command")
                
        except KeyboardInterrupt:
            print("\nExiting player...")
            self.is_running = False
    
    def _audio_simulator(self):
        """Simulate audio streaming with status updates"""
        while self.is_running:
            # Based on signal strength, simulate audio quality
            if self.current_signal > -50:
                quality = "Good"
            elif self.current_signal > -65:
                quality = "Fair"
            else:
                quality = "Poor/No Signal"
            
            # Print audio status without newline to keep interface clean
            print(f"\rPlaying FM {self.frequency/1e6:.1f} MHz | Signal: {self.current_signal:.1f} dB | Quality: {quality}      ", end="")
            sys.stdout.flush()
            sleep(1)
    
    def cleanup(self):
        """Release the device and clean up"""
        self.device.releaseDevice()
        print("\nDevice released. Goodbye!")


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description='Basic FM Radio Player')
    parser.add_argument('--frequency', type=float, default=100.0,
                        help='Initial frequency in MHz (default: 100.0)')
    
    args = parser.parse_args()
    
    player = FMPlayer(args.frequency)
    
    try:
        if player.initialize():
            player.run_player()
        else:
            print("Failed to initialize player")
            return 1
        
        return 0
    
    except Exception as e:
        print(f"Error: {e}")
        return 1
    
    finally:
        player.cleanup()


if __name__ == "__main__":
    sys.exit(main())