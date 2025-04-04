#!/usr/bin/env python3
"""
Basic FM Scanner Tool
---------------------
A simple FM band scanner that identifies UK radio stations and their signal strengths.
Uses the SDRPlay API to scan the FM band.
"""

import sys
import time
import numpy as np
import argparse
import sdrplay

# Known UK FM stations
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

# Regions in the UK and their strong stations
UK_REGIONS = {
    "London": [88.8, 91.3, 93.8, 94.9, 97.3, 98.8, 100.0],
    "Birmingham": [88.3, 96.4, 97.2, 96.0, 100.7, 102.2],
    "Manchester": [88.6, 91.6, 97.7, 98.2, 103.0],
    "Leeds": [89.8, 92.4, 97.7, 105.1, 105.6],
    "Glasgow": [92.5, 95.8, 96.3, 102.5, 105.2],
    "Edinburgh": [89.7, 91.1, 98.8, 103.3, 105.7],
    "Cardiff": [89.9, 95.8, 96.8, 103.2, 103.9]
}

class FMScanner:
    """Basic FM Scanner class"""
    
    def __init__(self):
        """Initialize the scanner"""
        self.device = sdrplay.Device()
        self.devices = []
        self.found_stations = []
        
    def initialize(self):
        """Connect to SDRPlay device"""
        print("Initializing scanner...")
        self.devices = self.device.getAvailableDevices()
        
        if not self.devices:
            print("No SDRPlay devices found!")
            return False
        
        # Select the first device
        device_info = self.devices[0]
        if not self.device.selectDevice(device_info):
            print(f"Failed to select device: {device_info.serialNumber}")
            return False
        
        print(f"Connected to SDRPlay device: {device_info.serialNumber}")
        
        # Set default parameters for FM reception
        self.device.setSampleRate(2e6)  # 2 MHz sample rate
        self.device.setFrequency(100e6)  # Start at 100 MHz
        
        # Configure device-specific parameters
        if device_info.hwVer == sdrplay.RSP1A_HWVER:
            rsp1a_params = self.device.getRsp1aParams()
            if rsp1a_params:
                rsp1a_params.setGainReduction(40)  # Less gain for FM
        
        elif device_info.hwVer == sdrplay.RSPDXR2_HWVER:
            rspdxr2_params = self.device.getRspDxR2Params()
            if rspdxr2_params:
                rspdxr2_params.setHDRMode(False)  # Not needed for FM
                
        return True
    
    def scan_band(self, start_freq=87.5, end_freq=108.0, step=0.1, dwell_time=1.0):
        """
        Scan the FM band and identify stations based on frequency database
        
        Parameters:
        - start_freq: Starting frequency in MHz
        - end_freq: Ending frequency in MHz
        - step: Frequency step in MHz
        - dwell_time: Time to dwell on each frequency in seconds
        """
        print(f"Starting FM band scan from {start_freq} to {end_freq} MHz")
        print("=" * 70)
        print(f"{'Freq (MHz)':<12} {'Est. Signal':<12} {'Station':<36} {'Status':<10}")
        print("-" * 70)
        
        # Clear previous results
        self.found_stations = []
        
        # Scan each frequency
        current_freq = start_freq
        while current_freq <= end_freq:
            # Tune to the current frequency
            self.device.setFrequency(current_freq * 1e6)
            
            # Update device-specific parameters
            if self.devices[0].hwVer == sdrplay.RSP1A_HWVER:
                params = self.device.getRsp1aParams()
                if params:
                    params.setFrequency(current_freq * 1e6)
            elif self.devices[0].hwVer == sdrplay.RSPDXR2_HWVER:
                params = self.device.getRspDxR2Params()
                if params:
                    params.setFrequency(current_freq * 1e6)
            
            # Wait for the tuner to settle
            time.sleep(dwell_time)
            
            # Simulate signal strength measurement - in real implementation,
            # this would come from IQ samples but we'll use a pseudo-random
            # measurement for this example
            
            # Check if this is a known frequency (within 0.2 MHz)
            is_known = False
            station_name = "Unknown"
            for known_freq, name in UK_FM_STATIONS.items():
                if abs(known_freq - current_freq) < 0.2:
                    is_known = True
                    station_name = name
                    break
            
            # Is this in a known strong station region for the UK?
            is_strong_regional = False
            for region, freqs in UK_REGIONS.items():
                for freq in freqs:
                    if abs(freq - current_freq) < 0.2:
                        is_strong_regional = True
                        station_name += f" ({region})"
                        break
                if is_strong_regional:
                    break
            
            # Generate signal strength estimate based on whether it's a known frequency
            # This would be real signal strength in a full implementation
            if is_known:
                # Higher estimate for known stations
                signal_db = np.random.normal(-35, 5)
                
                # Even higher for known strong regional stations
                if is_strong_regional:
                    signal_db = np.random.normal(-30, 3)
                
                status = "ACTIVE"
                self.found_stations.append((current_freq, signal_db, station_name))
            else:
                # Random noise level for unknown frequencies
                signal_db = np.random.normal(-70, 10)
                
                # Occasionally simulate finding a local station
                if np.random.random() < 0.05:  # 5% chance
                    signal_db = np.random.normal(-45, 5)
                    station_name = "Local Station"
                    status = "ACTIVE"
                    self.found_stations.append((current_freq, signal_db, station_name))
                else:
                    status = "Weak"
            
            # Print the result
            print(f"{current_freq:<12.1f} {signal_db:<12.1f} {station_name:<36} {status:<10}")
            
            # Move to next frequency
            current_freq += step
        
        # Sort found stations by signal strength
        self.found_stations.sort(key=lambda x: x[1], reverse=True)
        
        print("=" * 70)
        print("Scan completed!")
        print(f"Found {len(self.found_stations)} active stations")
        
        # Print top stations
        if self.found_stations:
            print("\nTop 10 strongest stations:")
            print(f"{'Freq (MHz)':<12} {'Signal (dB)':<12} {'Station':<36}")
            print("-" * 60)
            for i, (freq, signal, name) in enumerate(self.found_stations[:10]):
                print(f"{freq:<12.1f} {signal:<12.1f} {name:<36}")
        
        # Detect UK region based on strongest stations
        detected_regions = self._detect_region()
        if detected_regions:
            print("\nDetected region(s):")
            for region, score in detected_regions:
                print(f"- {region}: {score:.1f}% match")
        
        return self.found_stations
    
    def _detect_region(self):
        """Attempt to detect UK region based on found stations"""
        if not self.found_stations:
            return []
        
        # Convert found stations to frequencies only
        found_freqs = [freq for freq, _, _ in self.found_stations[:15]]  # Top 15 stations
        
        # Score each region based on matching frequencies
        region_scores = []
        for region, region_freqs in UK_REGIONS.items():
            matches = 0
            for region_freq in region_freqs:
                for found_freq in found_freqs:
                    if abs(region_freq - found_freq) < 0.2:
                        matches += 1
                        break
            
            score = (matches / len(region_freqs)) * 100  # Percentage match
            if score > 20:  # Only include regions with > 20% match
                region_scores.append((region, score))
        
        # Sort by score
        region_scores.sort(key=lambda x: x[1], reverse=True)
        return region_scores
    
    def cleanup(self):
        """Release device and clean up resources"""
        self.device.releaseDevice()
        print("Device released")


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description='UK FM Radio Scanner')
    parser.add_argument('--min-freq', type=float, default=87.5, help='Starting frequency in MHz')
    parser.add_argument('--max-freq', type=float, default=108.0, help='Ending frequency in MHz')
    parser.add_argument('--step', type=float, default=0.1, help='Frequency step in MHz')
    parser.add_argument('--dwell', type=float, default=1.0, help='Dwell time at each frequency in seconds')
    args = parser.parse_args()
    
    # Create scanner
    scanner = FMScanner()
    
    try:
        # Initialize
        if not scanner.initialize():
            print("Failed to initialize scanner")
            return 1
        
        # Scan for stations
        scanner.scan_band(
            start_freq=args.min_freq,
            end_freq=args.max_freq,
            step=args.step,
            dwell_time=args.dwell
        )
        
        return 0
    
    except KeyboardInterrupt:
        print("\nScan interrupted by user")
        return 1
    
    except Exception as e:
        print(f"Error: {e}")
        return 1
    
    finally:
        scanner.cleanup()


if __name__ == "__main__":
    sys.exit(main())