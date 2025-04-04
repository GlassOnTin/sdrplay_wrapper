#!/usr/bin/env python3
"""
UK FM Station Scanner
--------------------
Scans for UK FM radio stations and reports their signal strengths.
Uses the SDRPlay API to scan the FM band for active stations.

Requires the radio_receiver.py module.
"""

import sdrplay
import numpy as np
import scipy.signal as signal
import argparse
import time
import threading
from radio_receiver import RadioReceiver, Band, DemodMode
import sys

# UK FM frequencies in MHz
# Based on common UK national and regional FM broadcast frequencies
UK_FM_STATIONS = {
    87.5: "Local/Community",
    87.6: "BBC Radio Local",
    87.7: "Local/Community",
    87.8: "Community",
    88.0: "Local/Community",
    88.1: "Local/Community",
    88.2: "Local/Community",
    88.3: "Local/Community",
    88.4: "Local/Community",
    88.5: "Local/Commercial",
    88.6: "BBC Radio Local",
    88.8: "Local/Commercial",
    89.0: "Local/Community",
    89.1: "BBC Radio 2",
    89.7: "BBC Radio 2",
    90.0: "Community Radio",
    90.1: "Local/Commercial",
    90.2: "Local/Commercial",
    90.3: "Local/Commercial",
    90.4: "Local/Commercial",
    90.5: "Local/Commercial",
    90.7: "BBC Radio 3",
    91.0: "BBC Radio 3",
    91.3: "BBC Radio 3",
    91.5: "Local/Commercial",
    91.9: "Local/Commercial",
    92.1: "Local/Commercial",
    92.4: "BBC Radio 3",
    92.5: "BBC Radio Scotland",
    92.6: "Local/Commercial",
    92.9: "Local/Commercial",
    93.0: "Local/Commercial",
    93.2: "Local/Commercial",
    93.3: "Local/Commercial",
    93.5: "BBC Radio 4",
    93.8: "BBC Radio 4",
    94.2: "Local/Commercial",
    94.3: "Local/Commercial",
    94.6: "BBC Radio 3",
    94.9: "BBC Radio 4",
    95.3: "BBC Radio Scotland",
    95.4: "Local/Commercial",
    95.7: "Local/Commercial",
    95.8: "BBC Radio 2",
    96.0: "Classic FM",
    96.1: "Local/Commercial",
    96.2: "Local/Commercial",
    96.3: "Classic FM",
    96.5: "Local/Commercial",
    96.7: "BBC Radio 1",
    96.9: "Local/Commercial",
    97.1: "Local/Commercial",
    97.3: "Local/Commercial",
    97.4: "Local/Commercial",
    97.6: "BBC Radio 1",
    97.7: "Local/Commercial",
    97.9: "Local/Commercial",
    98.0: "Local/Commercial",
    98.2: "Local/Commercial",
    98.4: "BBC Radio 1",
    98.6: "Local/Commercial",
    98.8: "BBC Radio 1",
    99.0: "Local/Commercial",
    99.3: "Local/Commercial",
    99.5: "Classic FM",
    99.8: "Local/Commercial",
    100.0: "Local/Commercial",
    100.1: "Local/Commercial",
    100.4: "Classic FM",
    100.6: "Local/Commercial",
    100.9: "Local/Commercial",
    101.0: "Local/Commercial",
    101.1: "Local/Commercial",
    101.2: "Local/Commercial",
    101.4: "Local/Commercial",
    101.5: "Local/Commercial",
    101.7: "Local/Commercial", 
    101.9: "Local/Commercial",
    102.0: "Local/Commercial",
    102.2: "BBC Radio 2",
    102.6: "Local/Commercial",
    102.8: "Local/Commercial",
    103.0: "Local/Commercial",
    103.2: "Local/Commercial",
    103.3: "Local/Commercial",
    103.6: "Local/Commercial",
    103.9: "Local/Commercial",
    104.1: "Local/Commercial",
    104.3: "Local/Commercial",
    104.4: "Local/Commercial",
    104.6: "Classic FM",
    104.9: "Local/Commercial",
    105.2: "Local/Commercial",
    105.4: "Local/Commercial",
    105.7: "Local/Commercial",
    105.8: "Local/Commercial",
    106.0: "Local/Commercial",
    106.1: "Local/Commercial",
    106.2: "Local/Commercial",
    106.5: "Local/Commercial",
    106.8: "Local/Commercial",
    107.0: "Local/Commercial",
    107.1: "Local/Commercial",
    107.2: "Local/Commercial",
    107.4: "Local/Community",
    107.6: "Local/Community",
    107.8: "Local/Community",
    107.9: "Local/Community"
}

class FMScanner:
    """Scans for FM stations in the UK band"""
    
    def __init__(self, scan_step=0.1, min_signal_db=-60, dwell_time=0.5):
        """
        Initialize the FM scanner
        
        Parameters:
        - scan_step: Frequency step in MHz for scanning
        - min_signal_db: Minimum signal level in dB to consider a station detected
        - dwell_time: Time to dwell on each frequency in seconds
        """
        self.radio = RadioReceiver()
        self.scan_step = scan_step * 1e6  # Convert to Hz
        self.min_signal_db = min_signal_db
        self.dwell_time = dwell_time
        self.is_scanning = False
        self.stop_scan = False
        self.scan_results = []
        self.scan_thread = None
        self.lock = threading.Lock()
    
    def initialize(self):
        """Initialize the scanner and connect to the SDR device"""
        return self.radio.initialize()
    
    def start_scan(self, start_freq=87.5, end_freq=108.0):
        """
        Start scanning the FM band
        
        Parameters:
        - start_freq: Start frequency in MHz
        - end_freq: End frequency in MHz
        """
        if self.is_scanning:
            print("Scanner is already running!")
            return False
        
        self.stop_scan = False
        self.scan_results = []
        self.is_scanning = True
        
        # Create and start the scan thread
        self.scan_thread = threading.Thread(
            target=self._scan_fm_band,
            args=(start_freq * 1e6, end_freq * 1e6)
        )
        self.scan_thread.daemon = True
        self.scan_thread.start()
        
        return True
    
    def stop_scanning(self):
        """Stop the scan process"""
        self.stop_scan = True
        if self.scan_thread and self.scan_thread.is_alive():
            self.scan_thread.join(timeout=3.0)
        self.is_scanning = False
    
    def _scan_fm_band(self, start_freq, end_freq):
        """Perform the frequency scan (internal method)"""
        print(f"Starting FM band scan from {start_freq/1e6:.1f} to {end_freq/1e6:.1f} MHz")
        print("=" * 60)
        print(f"{'Freq (MHz)':<12} {'Signal (dB)':<12} {'Station':<36}")
        print("-" * 60)
        
        # Set radio to FM mode
        self.radio.set_band(Band.FM)
        self.radio.current_mode = DemodMode.FM
        
        # Ensure radio is streaming
        if not self.radio.running:
            self.radio.start()
        
        # Perform the scan
        current_freq = start_freq
        while current_freq <= end_freq and not self.stop_scan:
            # Tune to the current frequency
            self.radio.tune(current_freq)
            
            # Wait for signal to stabilize
            time.sleep(self.dwell_time)
            
            # Get the signal level
            signal_level = self.radio.signal_level
            
            # Check if we detected a station
            if signal_level > self.min_signal_db:
                # Find closest known frequency
                freq_mhz = round(current_freq / 1e6, 1)
                closest_freq = self._find_closest_frequency(freq_mhz)
                station_name = UK_FM_STATIONS.get(closest_freq, "Unknown Station")
                
                # Store and print the result
                result = {
                    'frequency': freq_mhz,
                    'signal_level': signal_level,
                    'station': station_name
                }
                
                with self.lock:
                    self.scan_results.append(result)
                
                print(f"{freq_mhz:<12.1f} {signal_level:<12.1f} {station_name:<36}")
            
            # Move to the next frequency
            current_freq += self.scan_step
        
        # Sort results by signal strength
        with self.lock:
            self.scan_results.sort(key=lambda x: x['signal_level'], reverse=True)
        
        print("=" * 60)
        print("Scan completed!")
        print(f"Found {len(self.scan_results)} stations")
        
        # Print top 10 strongest stations
        if self.scan_results:
            print("\nTop 10 strongest stations:")
            print(f"{'Freq (MHz)':<12} {'Signal (dB)':<12} {'Station':<36}")
            print("-" * 60)
            for i, result in enumerate(self.scan_results[:10]):
                print(f"{result['frequency']:<12.1f} {result['signal_level']:<12.1f} {result['station']:<36}")
        
        self.is_scanning = False
    
    def _find_closest_frequency(self, frequency):
        """Find the closest known frequency to the detected one"""
        known_freqs = list(UK_FM_STATIONS.keys())
        closest = min(known_freqs, key=lambda x: abs(x - frequency))
        
        # Only return if within 0.2 MHz
        if abs(closest - frequency) <= 0.2:
            return closest
        return frequency
    
    def get_results(self):
        """Get the scan results"""
        with self.lock:
            return self.scan_results.copy()
    
    def cleanup(self):
        """Clean up resources"""
        if self.is_scanning:
            self.stop_scanning()
        self.radio.cleanup()


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description="UK FM Radio Station Scanner")
    parser.add_argument("--min-freq", type=float, default=87.5, help="Starting frequency in MHz")
    parser.add_argument("--max-freq", type=float, default=108.0, help="Ending frequency in MHz")
    parser.add_argument("--step", type=float, default=0.1, help="Frequency step in MHz")
    parser.add_argument("--min-signal", type=float, default=-60, help="Minimum signal level in dB")
    parser.add_argument("--dwell", type=float, default=0.5, help="Dwell time at each frequency in seconds")
    args = parser.parse_args()
    
    scanner = FMScanner(
        scan_step=args.step,
        min_signal_db=args.min_signal,
        dwell_time=args.dwell
    )
    
    try:
        # Initialize scanner
        if not scanner.initialize():
            print("Failed to initialize scanner. Exiting.")
            return 1
        
        # Start scanning
        scanner.start_scan(args.min_freq, args.max_freq)
        
        # Wait for scan to complete
        while scanner.is_scanning:
            time.sleep(0.5)
        
        return 0
    
    except KeyboardInterrupt:
        print("\nScan interrupted by user.")
        return 1
    
    finally:
        scanner.cleanup()


if __name__ == "__main__":
    sys.exit(main())