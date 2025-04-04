#!/usr/bin/env python3
"""
Sony ICF-2010 Inspired Multi-band Radio Receiver
------------------------------------------------
A command-line software-defined radio receiver for LW, AM, FM, Airband, and SW bands,
inspired by the legendary Sony ICF-2010 (ICF-2001D) receiver from the 1980s.

Requires:
- SDRPlay Python wrapper
- NumPy
- SciPy
"""

import sdrplay
import numpy as np
import scipy.signal as signal
import argparse
import time
import threading
import curses
import sys
import os
from enum import Enum, auto

# Band definitions with frequency ranges (in Hz)
class Band(Enum):
    LW = auto()    # Longwave: 150 kHz - 530 kHz
    AM = auto()    # Medium Wave: 530 kHz - 1700 kHz
    SW = auto()    # Shortwave: 1700 kHz - 30 MHz
    FM = auto()    # FM broadcast: 88 MHz - 108 MHz
    AIR = auto()   # Airband: 118 MHz - 137 MHz

# Band frequency ranges (Hz)
BAND_RANGES = {
    Band.LW: (150e3, 530e3),
    Band.AM: (530e3, 1700e3),
    Band.SW: (1700e3, 30e6),
    Band.FM: (88e6, 108e6),
    Band.AIR: (118e6, 137e6)
}

# Default frequencies for each band (Hz)
DEFAULT_FREQUENCIES = {
    Band.LW: 198e3,     # BBC Radio 4 LW
    Band.AM: 1000e3,    # 1000 kHz
    Band.SW: 9500e3,    # 9500 kHz (31m band)
    Band.FM: 100e6,     # 100 MHz
    Band.AIR: 125.65e6  # Common air traffic frequency
}

# Parameters for different bands
BAND_PARAMS = {
    Band.LW: {"sample_rate": 1e6, "bandwidth": 5.0, "mode": "AM"},
    Band.AM: {"sample_rate": 1e6, "bandwidth": 10.0, "mode": "AM"},
    Band.SW: {"sample_rate": 2e6, "bandwidth": 5.0, "mode": "AM"},
    Band.FM: {"sample_rate": 2e6, "bandwidth": 200.0, "mode": "FM"},
    Band.AIR: {"sample_rate": 1e6, "bandwidth": 12.5, "mode": "AM"}
}

class DemodMode(Enum):
    AM = auto()
    FM = auto()
    USB = auto()  # Upper Sideband
    LSB = auto()  # Lower Sideband
    CW = auto()   # Continuous Wave (Morse)
    SYNC = auto()  # Synchronous AM

class RadioReceiver:
    def __init__(self):
        # Initialize SDRPlay device
        self.device = sdrplay.Device()
        self.devices = []
        self.current_band = Band.FM
        self.current_mode = DemodMode.FM
        self.current_frequency = DEFAULT_FREQUENCIES[self.current_band]
        self.current_volume = 0.5  # 0.0-1.0 scale
        self.running = False
        self.signal_level = 0
        self.signal_quality = 0
        self.buffer = []
        self.buffer_size = 10  # Number of sample blocks to keep
        self.lock = threading.Lock()
        self.ui_refresh_rate = 0.2  # seconds
        self.tuning_step = 1000  # Hz
        
        # Synchronous detector variables (for SYNC mode)
        self.sync_pll_freq = 0.0
        self.sync_pll_phase = 0.0
        
        # Settings that can be adjusted
        self.agc_enabled = True
        self.agc_setpoint = -30
        self.squelch_level = -60  # dBFS
        self.squelch_enabled = False
        
        # Callbacks
        self.stream_cb = None
        self.gain_cb = None
        self.power_cb = None
    
    def initialize(self):
        """Initialize the radio and connect to first available device"""
        # Get available devices
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
        
        # Set up callbacks
        self.stream_cb = self.StreamHandler(self)
        self.gain_cb = self.GainHandler(self)
        self.power_cb = self.PowerHandler(self)
        
        self.device.registerStreamCallback(self.stream_cb)
        self.device.registerGainCallback(self.gain_cb)
        self.device.registerPowerOverloadCallback(self.power_cb)
        
        # Set initial band
        self.set_band(Band.FM)
        
        return True
    
    def set_band(self, band):
        """Set the radio to the specified band"""
        self.current_band = band
        self.current_frequency = DEFAULT_FREQUENCIES[band]
        self.current_mode = DemodMode[BAND_PARAMS[band]["mode"]]
        params = BAND_PARAMS[band]
        
        # Adjust SDR parameters for the band
        self.device.setSampleRate(params["sample_rate"])
        self.device.setFrequency(self.current_frequency)
        
        # Configure device-specific parameters
        device_info = self.devices[0]
        
        # For RSP1A
        if device_info.hwVer == sdrplay.RSP1A_HWVER:
            rsp1a_params = self.device.getRsp1aParams()
            if rsp1a_params:
                rsp1a_params.setFrequency(self.current_frequency)
                rsp1a_params.setSampleRate(params["sample_rate"])
                # Set appropriate gain for the band
                if band in [Band.FM, Band.AIR]:
                    rsp1a_params.setGainReduction(40)  # VHF bands need less gain
                else:
                    rsp1a_params.setGainReduction(20)  # More gain for LW/MW/SW
        
        # For RSPdxR2
        elif device_info.hwVer == sdrplay.RSPDXR2_HWVER:
            rspdxr2_params = self.device.getRspDxR2Params()
            if rspdxr2_params:
                rspdxr2_params.setFrequency(self.current_frequency)
                rspdxr2_params.setSampleRate(params["sample_rate"])
                # Enable HDR mode for LW/MW/SW for better performance
                if band in [Band.LW, Band.AM, Band.SW]:
                    rspdxr2_params.setHDRMode(True)
                else:
                    rspdxr2_params.setHDRMode(False)
        
        # Set tuning step based on the band
        if band == Band.FM:
            self.tuning_step = 100e3  # 100 kHz for FM
        elif band == Band.AIR:
            self.tuning_step = 25e3   # 25 kHz for Airband
        elif band == Band.AM:
            self.tuning_step = 10e3   # 10 kHz for AM
        elif band == Band.SW:
            self.tuning_step = 5e3    # 5 kHz for SW
        else:
            self.tuning_step = 1e3    # 1 kHz for LW
        
        return True
    
    def tune(self, frequency):
        """Tune to a specific frequency in the current band"""
        # Check if frequency is within the current band
        band_min, band_max = BAND_RANGES[self.current_band]
        if frequency < band_min:
            frequency = band_min
        elif frequency > band_max:
            frequency = band_max
            
        self.current_frequency = frequency
        self.device.setFrequency(frequency)
        
        # Update device-specific parameters
        device_info = self.devices[0]
        if device_info.hwVer == sdrplay.RSP1A_HWVER:
            rsp1a_params = self.device.getRsp1aParams()
            if rsp1a_params:
                rsp1a_params.setFrequency(frequency)
        elif device_info.hwVer == sdrplay.RSPDXR2_HWVER:
            rspdxr2_params = self.device.getRspDxR2Params()
            if rspdxr2_params:
                rspdxr2_params.setFrequency(frequency)
        
        return True
    
    def start(self):
        """Start radio reception"""
        if not self.running:
            if self.device.startStreaming():
                self.running = True
                return True
            else:
                print("Failed to start streaming")
                return False
        return True

    def stop(self):
        """Stop radio reception"""
        if self.running:
            if self.device.stopStreaming():
                self.running = False
                return True
            else:
                print("Failed to stop streaming")
                return False
        return True
    
    def cleanup(self):
        """Clean up resources"""
        if self.running:
            self.stop()
        self.device.releaseDevice()
    
    def am_demodulate(self, i_data, q_data):
        """AM demodulation"""
        # Calculate the magnitude of complex samples
        amplitude = np.sqrt(i_data**2 + q_data**2)
        # Remove DC component
        return amplitude - np.mean(amplitude)
    
    def fm_demodulate(self, i_data, q_data):
        """FM demodulation using phase differential method"""
        # Create complex signal
        complex_signal = i_data + 1j * q_data
        
        # Phase differential demodulation
        product = np.conj(complex_signal[:-1]) * complex_signal[1:]
        phase_diff = np.angle(product)
        
        # Scale and return
        audio = phase_diff * 0.5  # Scale factor
        
        # Add a zero to maintain the same length
        return np.append(audio, 0)
    
    def ssb_demodulate(self, i_data, q_data, mode=DemodMode.USB):
        """SSB demodulation (USB or LSB)"""
        # Create complex signal
        complex_signal = i_data + 1j * q_data
        
        # Create an analytic signal with a Hilbert transform
        if mode == DemodMode.USB:
            # For USB, keep only positive frequencies
            analytic = signal.hilbert(np.real(complex_signal))
        else:
            # For LSB, keep only negative frequencies (conjugate)
            analytic = np.conj(signal.hilbert(np.real(complex_signal)))
        
        # Return the real part
        return np.real(analytic)
    
    def sync_am_demodulate(self, i_data, q_data):
        """Synchronous AM demodulation with PLL"""
        # Simple PLL-based synchronous AM demodulation
        complex_signal = i_data + 1j * q_data
        
        # Phase detector
        phase_error = np.angle(complex_signal)
        
        # Update PLL
        self.sync_pll_freq += 0.01 * np.mean(phase_error)
        self.sync_pll_phase += self.sync_pll_freq
        
        # Generate carrier
        carrier = np.exp(1j * self.sync_pll_phase)
        
        # Multiply by carrier
        demodulated = np.real(complex_signal * carrier)
        
        return demodulated - np.mean(demodulated)  # Remove DC
    
    def cw_demodulate(self, i_data, q_data):
        """CW (Morse) demodulation"""
        # Create a beat frequency oscillator (BFO) at 800 Hz
        bfo_freq = 800  # Hz
        sample_rate = BAND_PARAMS[self.current_band]["sample_rate"]
        t = np.arange(len(i_data)) / sample_rate
        bfo = np.exp(2j * np.pi * bfo_freq * t)
        
        # Mix with input signal
        complex_signal = i_data + 1j * q_data
        mixed = complex_signal * bfo
        
        # Return real part
        return np.real(mixed)
    
    def process_audio(self, audio):
        """Process audio samples (filtering, squelch, etc.)"""
        # Check signal level for squelch
        level_db = 20 * np.log10(np.std(audio) + 1e-10)
        self.signal_level = level_db
        
        # Apply squelch if enabled
        if self.squelch_enabled and level_db < self.squelch_level:
            return np.zeros_like(audio)
        
        # Apply volume control
        audio = audio * self.current_volume
        
        # Limit to prevent clipping
        audio = np.clip(audio, -1.0, 1.0)
        
        return audio
    
    def demodulate(self, i_data, q_data):
        """Demodulate I/Q data based on current mode"""
        if self.current_mode == DemodMode.AM:
            audio = self.am_demodulate(i_data, q_data)
        elif self.current_mode == DemodMode.FM:
            audio = self.fm_demodulate(i_data, q_data)
        elif self.current_mode == DemodMode.USB:
            audio = self.ssb_demodulate(i_data, q_data, DemodMode.USB)
        elif self.current_mode == DemodMode.LSB:
            audio = self.ssb_demodulate(i_data, q_data, DemodMode.LSB)
        elif self.current_mode == DemodMode.CW:
            audio = self.cw_demodulate(i_data, q_data)
        elif self.current_mode == DemodMode.SYNC:
            audio = self.sync_am_demodulate(i_data, q_data)
        else:
            audio = self.am_demodulate(i_data, q_data)
        
        return self.process_audio(audio)
    
    # Callback Handlers
    class StreamHandler(sdrplay.StreamCallbackHandler):
        def __init__(self, receiver):
            # For abstract classes in SWIG, don't call the parent constructor
            self.receiver = receiver
        
        def handleStreamData(self, xi, xq, numSamples):
            # Convert to numpy arrays for easier processing
            i_data = np.array(xi[:numSamples])
            q_data = np.array(xq[:numSamples])
            
            # Calculate signal quality metrics
            signal_power = np.mean(i_data**2 + q_data**2)
            self.receiver.signal_level = 10 * np.log10(signal_power + 1e-10)
            
            # Store complex samples in buffer for processing
            with self.receiver.lock:
                complex_samples = i_data + 1j * q_data
                self.receiver.buffer.append(complex_samples)
                if len(self.receiver.buffer) > self.receiver.buffer_size:
                    self.receiver.buffer.pop(0)  # Remove oldest data
            
            # For real-time audio processing, we would demodulate here,
            # but this example focuses on the command-line interface without audio output

    class GainHandler(sdrplay.GainCallbackHandler):
        def __init__(self, receiver):
            # For abstract classes in SWIG, don't call the parent constructor
            self.receiver = receiver
        
        def handleGainChange(self, gRdB, lnaGRdB, currGain):
            # This is called when AGC adjusts the gain
            pass

    class PowerHandler(sdrplay.PowerOverloadCallbackHandler):
        def __init__(self, receiver):
            # For abstract classes in SWIG, don't call the parent constructor
            self.receiver = receiver
        
        def handlePowerOverload(self, isOverloaded):
            # Notifies when signal is too strong (overloaded)
            if isOverloaded:
                print("WARNING: Signal overload detected!")


class RadioCLI:
    """Command-line interface for the radio receiver"""
    
    def __init__(self, radio):
        self.radio = radio
        self.running = False
        self.screen = None
    
    def start(self):
        """Start the CLI interface"""
        try:
            self.screen = curses.initscr()
            curses.noecho()
            curses.cbreak()
            self.screen.keypad(True)
            curses.curs_set(0)  # Hide cursor
            curses.start_color()
            curses.use_default_colors()
            
            # Initialize color pairs
            curses.init_pair(1, curses.COLOR_GREEN, -1)  # Green
            curses.init_pair(2, curses.COLOR_YELLOW, -1)  # Yellow
            curses.init_pair(3, curses.COLOR_RED, -1)    # Red
            curses.init_pair(4, curses.COLOR_CYAN, -1)   # Cyan for headers
            curses.init_pair(5, curses.COLOR_WHITE, curses.COLOR_BLUE)  # For selected items
            
            self.running = True
            self.radio.start()
            
            self.main_loop()
        
        finally:
            self.cleanup()
    
    def cleanup(self):
        """Clean up curses interface"""
        if self.screen:
            self.screen.keypad(False)
            curses.nocbreak()
            curses.echo()
            curses.endwin()
            self.running = False
        self.radio.cleanup()
    
    def main_loop(self):
        """Main UI loop"""
        # Get terminal size
        height, width = self.screen.getmaxyx()
        
        # Set non-blocking input with 200ms timeout
        self.screen.timeout(200)
        
        while self.running:
            # Clear screen
            self.screen.clear()
            
            # Draw the radio interface
            self.draw_header(width)
            self.draw_frequency_display(2, width)
            self.draw_band_selector(4, width)
            self.draw_mode_selector(6, width)
            self.draw_signal_indicators(8, width)
            self.draw_controls(12, width)
            
            # Refresh screen
            self.screen.refresh()
            
            # Get key input
            try:
                key = self.screen.getch()
                self.handle_key(key)
            except KeyboardInterrupt:
                break
    
    def draw_header(self, width):
        """Draw the header with the Sony model name"""
        title = "SONY ICF-2010 RECEIVER"
        x = (width - len(title)) // 2
        self.screen.addstr(0, x, title, curses.color_pair(4) | curses.A_BOLD)
    
    def draw_frequency_display(self, y, width):
        """Draw the frequency display"""
        freq_mhz = self.radio.current_frequency / 1e6
        if freq_mhz >= 1:
            freq_str = f"{freq_mhz:.5f} MHz"
        else:
            freq_khz = self.radio.current_frequency / 1e3
            freq_str = f"{freq_khz:.2f} kHz"
        
        display_width = 20
        x = (width - display_width) // 2
        self.screen.addstr(y, x, "┌" + "─" * (display_width-2) + "┐")
        self.screen.addstr(y+1, x, "│" + " " * (display_width-2) + "│")
        
        # Center the frequency text
        freq_x = x + (display_width - len(freq_str)) // 2
        self.screen.addstr(y+1, freq_x, freq_str, curses.A_BOLD | curses.color_pair(1))
        
        self.screen.addstr(y+2, x, "└" + "─" * (display_width-2) + "┘")
    
    def draw_band_selector(self, y, width):
        """Draw the band selector"""
        self.screen.addstr(y, 2, "BAND: ", curses.color_pair(4))
        
        bands = ["LW", "AM", "SW", "FM", "AIR"]
        band_width = 8
        spacing = 2
        start_x = 8
        
        for i, band_name in enumerate(bands):
            x = start_x + i * (band_width + spacing)
            if self.radio.current_band == Band[band_name]:
                # Highlight selected band
                self.screen.addstr(y, x, f" {band_name} ", curses.color_pair(5) | curses.A_BOLD)
            else:
                self.screen.addstr(y, x, f" {band_name} ")
    
    def draw_mode_selector(self, y, width):
        """Draw the demodulation mode selector"""
        self.screen.addstr(y, 2, "MODE: ", curses.color_pair(4))
        
        modes = ["AM", "FM", "USB", "LSB", "CW", "SYNC"]
        mode_width = 6
        spacing = 2
        start_x = 8
        
        for i, mode_name in enumerate(modes):
            x = start_x + i * (mode_width + spacing)
            if self.radio.current_mode == DemodMode[mode_name]:
                # Highlight selected mode
                self.screen.addstr(y, x, f" {mode_name} ", curses.color_pair(5) | curses.A_BOLD)
            else:
                self.screen.addstr(y, x, f" {mode_name} ")
    
    def draw_signal_indicators(self, y, width):
        """Draw signal strength and quality indicators"""
        # Signal level indicator
        self.screen.addstr(y, 2, "SIGNAL: ", curses.color_pair(4))
        
        # Map signal level to bars (-80 dBFS to -20 dBFS range)
        level = min(max(self.radio.signal_level, -80), -20)
        normalized = (level + 80) / 60  # 0.0 to 1.0 scale
        bars = int(normalized * 20)
        
        # Choose color based on signal strength
        if normalized > 0.7:
            color = curses.color_pair(1)  # Green for strong signal
        elif normalized > 0.3:
            color = curses.color_pair(2)  # Yellow for medium signal
        else:
            color = curses.color_pair(3)  # Red for weak signal
        
        self.screen.addstr(y, 10, "[" + "=" * bars + " " * (20 - bars) + "]", color)
        self.screen.addstr(y, 32, f"{level:.1f} dB", color)
        
        # Status indicators
        status_y = y + 1
        status_x = 2
        
        # AGC status
        agc_status = "ON" if self.radio.agc_enabled else "OFF"
        self.screen.addstr(status_y, status_x, f"AGC: {agc_status}")
        
        # Squelch status
        squelch_status = f"ON ({self.radio.squelch_level} dB)" if self.radio.squelch_enabled else "OFF"
        self.screen.addstr(status_y, status_x + 15, f"SQUELCH: {squelch_status}")
        
        # Streaming status
        streaming_status = "ACTIVE" if self.radio.running else "STOPPED"
        self.screen.addstr(status_y, status_x + 40, f"STREAMING: {streaming_status}")
    
    def draw_controls(self, y, width):
        """Draw the keyboard controls help"""
        self.screen.addstr(y, 2, "CONTROLS:", curses.color_pair(4) | curses.A_BOLD)
        self.screen.addstr(y+1, 2, "←/→: Tune frequency  ↑/↓: Change band  M: Change mode  S: Start/Stop")
        self.screen.addstr(y+2, 2, "+/-: Volume  A: Toggle AGC  Q: Toggle squelch  ESC/X: Exit")
    
    def handle_key(self, key):
        """Handle keyboard input"""
        if key == curses.KEY_LEFT:
            # Tune down
            self.radio.tune(self.radio.current_frequency - self.radio.tuning_step)
        
        elif key == curses.KEY_RIGHT:
            # Tune up
            self.radio.tune(self.radio.current_frequency + self.radio.tuning_step)
        
        elif key == curses.KEY_UP:
            # Switch to next band
            bands = list(Band)
            current_index = bands.index(self.radio.current_band)
            next_index = (current_index + 1) % len(bands)
            self.radio.set_band(bands[next_index])
        
        elif key == curses.KEY_DOWN:
            # Switch to previous band
            bands = list(Band)
            current_index = bands.index(self.radio.current_band)
            prev_index = (current_index - 1) % len(bands)
            self.radio.set_band(bands[prev_index])
        
        elif key == ord('m') or key == ord('M'):
            # Switch to next demodulation mode
            modes = list(DemodMode)
            current_index = modes.index(self.radio.current_mode)
            next_index = (current_index + 1) % len(modes)
            self.radio.current_mode = modes[next_index]
        
        elif key == ord('s') or key == ord('S'):
            # Start/stop streaming
            if self.radio.running:
                self.radio.stop()
            else:
                self.radio.start()
        
        elif key == ord('+') or key == ord('='):
            # Increase volume (max 1.0)
            self.radio.current_volume = min(1.0, self.radio.current_volume + 0.1)
        
        elif key == ord('-') or key == ord('_'):
            # Decrease volume (min 0.0)
            self.radio.current_volume = max(0.0, self.radio.current_volume - 0.1)
        
        elif key == ord('a') or key == ord('A'):
            # Toggle AGC
            self.radio.agc_enabled = not self.radio.agc_enabled
        
        elif key == ord('q') or key == ord('Q'):
            # Toggle squelch
            self.radio.squelch_enabled = not self.radio.squelch_enabled
        
        elif key == 27 or key == ord('x') or key == ord('X'):
            # Exit (ESC key or X)
            self.running = False


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="Sony ICF-2010 Inspired Multi-band Radio Receiver"
    )
    parser.add_argument("--frequency", type=float, help="Initial frequency in MHz")
    parser.add_argument("--band", choices=["LW", "AM", "SW", "FM", "AIR"], help="Initial band")
    parser.add_argument("--mode", choices=["AM", "FM", "USB", "LSB", "CW", "SYNC"], help="Demodulation mode")
    args = parser.parse_args()
    
    # Create and initialize radio
    radio = RadioReceiver()
    success = radio.initialize()
    
    if not success:
        print("Failed to initialize radio. Exiting.")
        return 1
    
    # Apply command line arguments if provided
    if args.band:
        radio.set_band(Band[args.band])
    
    if args.frequency:
        radio.tune(args.frequency * 1e6)  # Convert MHz to Hz
    
    if args.mode:
        radio.current_mode = DemodMode[args.mode]
    
    # Start the CLI
    cli = RadioCLI(radio)
    cli.start()
    
    return 0


if __name__ == "__main__":
    sys.exit(main())