#!/usr/bin/env python3
"""
FM Radio Command Line Interface
------------------------------
A command-line interface for the FM radio module.
This demonstrates the architecture for the SDRPlay wrapper with audio streaming.
"""

import os
import sys
import time
import argparse
import curses
import threading
from enum import Enum

# Add the project root directory to the Python path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

# Import our FM radio module
from sdrplay.fm_radio import FMRadio, DemodMode

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

class RadioState(Enum):
    MAIN = 0
    TUNING = 1
    STATION_LIST = 2
    SETTINGS = 3


class FMRadioCLI:
    """Command-line interface for the FM radio"""
    
    def __init__(self, radio):
        """Initialize the CLI with a radio instance"""
        self.radio = radio
        self.screen = None
        self.running = False
        self.current_state = RadioState.MAIN
        self.status_thread = None
        self.status_lock = threading.Lock()
        self.signal_level = -100.0
        self.station_list = list(UK_FM_STATIONS.items())
        self.station_index = 0
        self.scan_progress = 0
        self.preset_stations = []
        self.presets_changed = False
        self.volume_adjustment = False
        self.squelch_adjustment = False
        
    def start(self):
        """Start the CLI interface"""
        try:
            self.screen = curses.initscr()
            curses.noecho()
            curses.cbreak()
            curses.start_color()
            curses.use_default_colors()
            self.screen.keypad(True)
            curses.curs_set(0)  # Hide cursor
            
            # Initialize color pairs
            curses.init_pair(1, curses.COLOR_GREEN, -1)      # Green text
            curses.init_pair(2, curses.COLOR_YELLOW, -1)     # Yellow text
            curses.init_pair(3, curses.COLOR_RED, -1)        # Red text
            curses.init_pair(4, curses.COLOR_CYAN, -1)       # Cyan text
            curses.init_pair(5, curses.COLOR_WHITE, curses.COLOR_BLUE)  # Selected items
            
            # Start the radio
            if not self.radio.start():
                self.cleanup()
                print("Failed to start radio")
                return False
            
            # Start status update thread
            self.running = True
            self.status_thread = threading.Thread(target=self._status_updater, daemon=True)
            self.status_thread.start()
            
            # Main loop
            self._main_loop()
            
            return True
        
        except Exception as e:
            self.cleanup()
            print(f"Error: {e}")
            return False
        
        finally:
            self.cleanup()
    
    def cleanup(self):
        """Clean up resources"""
        if self.screen:
            self.screen.keypad(False)
            curses.nocbreak()
            curses.echo()
            curses.endwin()
        
        self.running = False
        
        if self.status_thread and self.status_thread.is_alive():
            self.status_thread.join(timeout=1.0)
        
        if self.radio:
            self.radio.stop()
            self.radio.close()
    
    def _status_updater(self):
        """Thread that updates signal levels and other status information"""
        while self.running:
            # Get signal level from radio
            level = self.radio.get_signal_level()
            
            with self.status_lock:
                self.signal_level = level
            
            # Update every 100ms
            time.sleep(0.1)
    
    def _main_loop(self):
        """Main interface loop"""
        self.screen.timeout(100)  # Non-blocking input with 100ms timeout
        
        while self.running:
            # Clear screen
            self.screen.clear()
            
            # Draw UI based on current state
            if self.current_state == RadioState.MAIN:
                self._draw_main_screen()
            elif self.current_state == RadioState.TUNING:
                self._draw_tuning_screen()
            elif self.current_state == RadioState.STATION_LIST:
                self._draw_station_list()
            elif self.current_state == RadioState.SETTINGS:
                self._draw_settings_screen()
            
            # Refresh screen
            self.screen.refresh()
            
            # Handle input
            try:
                key = self.screen.getch()
                if key != -1:  # -1 means no key was pressed (timeout)
                    self._handle_key(key)
            except KeyboardInterrupt:
                self.running = False
    
    def _draw_main_screen(self):
        """Draw the main radio screen"""
        # Get screen dimensions
        height, width = self.screen.getmaxyx()
        
        # Draw title
        title = "SONY ICF-2010 RADIO RECEIVER"
        x = (width - len(title)) // 2
        self.screen.addstr(0, x, title, curses.color_pair(4) | curses.A_BOLD)
        
        # Draw frequency display
        freq_mhz = self.radio.tuned_frequency / 1e6
        freq_str = f"{freq_mhz:.2f} MHz"
        
        display_width = 20
        display_x = (width - display_width) // 2
        self.screen.addstr(2, display_x, "┌" + "─" * (display_width-2) + "┐")
        self.screen.addstr(3, display_x, "│" + " " * (display_width-2) + "│")
        
        # Center the frequency text
        freq_x = display_x + (display_width - len(freq_str)) // 2
        self.screen.addstr(3, freq_x, freq_str, curses.A_BOLD | curses.color_pair(1))
        
        self.screen.addstr(4, display_x, "└" + "─" * (display_width-2) + "┘")
        
        # Identify known station
        known_station = self._get_station_name(freq_mhz)
        if known_station:
            station_str = f"Station: {known_station}"
            station_x = (width - len(station_str)) // 2
            self.screen.addstr(5, station_x, station_str, curses.color_pair(4))
        
        # Draw signal meter
        self._draw_signal_meter(7, width)
        
        # Draw mode (FM/AM/etc.)
        mode_str = f"Mode: {self.radio.demod_mode.name}"
        mode_x = (width - len(mode_str)) // 2
        self.screen.addstr(9, mode_x, mode_str)
        
        # Draw status indicators
        status_y = 11
        
        # Volume
        vol_percent = int(self.radio.volume * 100)
        vol_str = f"Volume: {vol_percent}%"
        self.screen.addstr(status_y, 5, vol_str)
        
        # Squelch
        squelch_str = f"Squelch: {'ON' if self.radio.squelch_enabled else 'OFF'}"
        if self.radio.squelch_enabled:
            squelch_str += f" ({self.radio.squelch_level:.0f} dB)"
        self.screen.addstr(status_y, width - 25, squelch_str)
        
        # Draw controls
        controls_y = height - 3
        self.screen.addstr(controls_y, 2, "CONTROLS:", curses.color_pair(4) | curses.A_BOLD)
        self.screen.addstr(controls_y + 1, 2, 
                         "F1-F4: Presets | T: Tune | L: Station List | S: Settings | Q: Quit")
    
    def _draw_tuning_screen(self):
        """Draw the tuning screen"""
        # Get screen dimensions
        height, width = self.screen.getmaxyx()
        
        # Draw title
        title = "TUNING MODE"
        x = (width - len(title)) // 2
        self.screen.addstr(0, x, title, curses.color_pair(4) | curses.A_BOLD)
        
        # Draw frequency display
        freq_mhz = self.radio.tuned_frequency / 1e6
        freq_str = f"{freq_mhz:.2f} MHz"
        
        display_width = 20
        display_x = (width - display_width) // 2
        self.screen.addstr(2, display_x, "┌" + "─" * (display_width-2) + "┐")
        self.screen.addstr(3, display_x, "│" + " " * (display_width-2) + "│")
        
        # Center the frequency text
        freq_x = display_x + (display_width - len(freq_str)) // 2
        self.screen.addstr(3, freq_x, freq_str, curses.A_BOLD | curses.color_pair(1))
        
        self.screen.addstr(4, display_x, "└" + "─" * (display_width-2) + "┘")
        
        # Draw tuning instructions
        self.screen.addstr(6, 5, "Use LEFT/RIGHT arrow keys to change frequency")
        self.screen.addstr(7, 5, "Press UP/DOWN to change tuning step")
        self.screen.addstr(8, 5, "Press ENTER to save frequency to a preset")
        self.screen.addstr(9, 5, "Press ESC to return to main screen")
        
        # Draw current tuning step
        step_khz = 100  # Default 100 kHz for FM
        step_str = f"Tuning Step: {step_khz} kHz"
        self.screen.addstr(11, 5, step_str)
        
        # Draw signal meter
        self._draw_signal_meter(13, width)
        
        # Known station
        known_station = self._get_station_name(freq_mhz)
        if known_station:
            station_str = f"Station: {known_station}"
            station_x = 5
            self.screen.addstr(15, station_x, station_str, curses.color_pair(4))
        
        # Draw bottom controls
        controls_y = height - 3
        self.screen.addstr(controls_y, 2, "CONTROLS:", curses.color_pair(4) | curses.A_BOLD)
        self.screen.addstr(controls_y + 1, 2, 
                         "LEFT/RIGHT: Tune | UP/DOWN: Change step | ENTER: Save | ESC: Back")
    
    def _draw_station_list(self):
        """Draw the station list screen"""
        # Get screen dimensions
        height, width = self.screen.getmaxyx()
        
        # Draw title
        title = "STATION LIST"
        x = (width - len(title)) // 2
        self.screen.addstr(0, x, title, curses.color_pair(4) | curses.A_BOLD)
        
        # Calculate visible stations
        max_visible = height - 8
        total_stations = len(self.station_list)
        
        # Ensure current index is valid
        if self.station_index < 0:
            self.station_index = 0
        elif self.station_index >= total_stations:
            self.station_index = total_stations - 1
        
        # Calculate start index for scrolling
        start_index = max(0, self.station_index - max_visible // 2)
        end_index = min(total_stations, start_index + max_visible)
        
        # Draw column headers
        self.screen.addstr(2, 5, "Frequency", curses.A_BOLD)
        self.screen.addstr(2, 20, "Station", curses.A_BOLD)
        
        # Draw station list
        for i, (freq, name) in enumerate(self.station_list[start_index:end_index], start=3):
            # Highlight selected station
            attrs = curses.color_pair(5) if i - 3 + start_index == self.station_index else 0
            
            self.screen.addstr(i, 5, f"{freq:.1f} MHz", attrs)
            self.screen.addstr(i, 20, name, attrs)
        
        # Draw scroll indicators if needed
        if start_index > 0:
            self.screen.addstr(2, width - 5, "▲")
        if end_index < total_stations:
            self.screen.addstr(max_visible + 2, width - 5, "▼")
        
        # Draw controls
        controls_y = height - 3
        self.screen.addstr(controls_y, 2, "CONTROLS:", curses.color_pair(4) | curses.A_BOLD)
        self.screen.addstr(controls_y + 1, 2, 
                         "UP/DOWN: Navigate | ENTER: Tune to station | ESC: Back")
    
    def _draw_settings_screen(self):
        """Draw the settings screen"""
        # Get screen dimensions
        height, width = self.screen.getmaxyx()
        
        # Draw title
        title = "SETTINGS"
        x = (width - len(title)) // 2
        self.screen.addstr(0, x, title, curses.color_pair(4) | curses.A_BOLD)
        
        # Draw settings
        settings_x = 5
        
        # Mode setting
        mode_str = f"Demodulation Mode: {self.radio.demod_mode.name}"
        self.screen.addstr(3, settings_x, mode_str)
        self.screen.addstr(4, settings_x + 5, "Press 'M' to change mode")
        
        # Volume setting
        vol_percent = int(self.radio.volume * 100)
        vol_str = f"Volume: {vol_percent}%"
        
        if self.volume_adjustment:
            # Highlight when adjusting
            self.screen.addstr(6, settings_x, vol_str, curses.color_pair(5) | curses.A_BOLD)
        else:
            self.screen.addstr(6, settings_x, vol_str)
            
        self.screen.addstr(7, settings_x + 5, "Press 'V' then LEFT/RIGHT to adjust")
        
        # Squelch setting
        squelch_str = f"Squelch: {'ON' if self.radio.squelch_enabled else 'OFF'}"
        if self.radio.squelch_enabled:
            squelch_str += f" ({self.radio.squelch_level:.0f} dB)"
            
        if self.squelch_adjustment:
            # Highlight when adjusting
            self.screen.addstr(9, settings_x, squelch_str, curses.color_pair(5) | curses.A_BOLD)
        else:
            self.screen.addstr(9, settings_x, squelch_str)
            
        self.screen.addstr(10, settings_x + 5, "Press 'Q' to toggle, then LEFT/RIGHT to adjust level")
        
        # Draw controls
        controls_y = height - 3
        self.screen.addstr(controls_y, 2, "CONTROLS:", curses.color_pair(4) | curses.A_BOLD)
        self.screen.addstr(controls_y + 1, 2, 
                         "M: Mode | V: Volume | Q: Squelch | ESC: Back")
    
    def _draw_signal_meter(self, y, width):
        """Draw a signal strength meter"""
        with self.status_lock:
            signal_level = self.signal_level
        
        self.screen.addstr(y, 5, "Signal: ", curses.color_pair(4))
        
        # Map signal level to bar length (-100 dB to -20 dB range)
        level = min(max(signal_level, -100), -20)
        normalized = (level + 100) / 80  # Scale to 0.0-1.0
        bar_width = 30
        bars = int(normalized * bar_width)
        
        # Choose color based on signal strength
        if normalized > 0.75:
            color = curses.color_pair(1)  # Green for strong signal
        elif normalized > 0.4:
            color = curses.color_pair(2)  # Yellow for medium signal
        else:
            color = curses.color_pair(3)  # Red for weak signal
        
        # Draw meter
        meter_x = 13
        self.screen.addstr(y, meter_x, "[" + "=" * bars + " " * (bar_width - bars) + "]", color)
        
        # Draw dB value
        self.screen.addstr(y, meter_x + bar_width + 3, f"{level:.1f} dB", color)
    
    def _get_station_name(self, freq_mhz):
        """Get station name for a frequency"""
        # Find closest known station (within 0.2 MHz)
        for known_freq, name in UK_FM_STATIONS.items():
            if abs(freq_mhz - known_freq) <= 0.2:
                return name
        return None
    
    def _handle_key(self, key):
        """Handle keyboard input"""
        if key == ord('q') or key == ord('Q'):
            self.running = False
            return
            
        if self.current_state == RadioState.MAIN:
            self._handle_main_keys(key)
        elif self.current_state == RadioState.TUNING:
            self._handle_tuning_keys(key)
        elif self.current_state == RadioState.STATION_LIST:
            self._handle_station_list_keys(key)
        elif self.current_state == RadioState.SETTINGS:
            self._handle_settings_keys(key)
    
    def _handle_main_keys(self, key):
        """Handle keys in main screen"""
        if key == ord('t') or key == ord('T'):
            # Enter tuning mode
            self.current_state = RadioState.TUNING
        
        elif key == ord('l') or key == ord('L'):
            # Show station list
            self.current_state = RadioState.STATION_LIST
        
        elif key == ord('s') or key == ord('S'):
            # Show settings
            self.current_state = RadioState.SETTINGS
        
        elif key in [curses.KEY_F1, curses.KEY_F2, curses.KEY_F3, curses.KEY_F4]:
            # Preset keys
            preset_index = key - curses.KEY_F1
            if preset_index < len(self.preset_stations):
                # Tune to preset
                preset_freq = self.preset_stations[preset_index]
                self.radio.tune(preset_freq)
    
    def _handle_tuning_keys(self, key):
        """Handle keys in tuning mode"""
        if key == 27:  # ESC key
            # Return to main screen
            self.current_state = RadioState.MAIN
        
        elif key == curses.KEY_LEFT:
            # Tune down
            step_khz = 100  # 100 kHz for FM
            self.radio.tune(self.radio.tuned_frequency - step_khz * 1000)
        
        elif key == curses.KEY_RIGHT:
            # Tune up
            step_khz = 100  # 100 kHz for FM
            self.radio.tune(self.radio.tuned_frequency + step_khz * 1000)
        
        elif key == 10:  # Enter key
            # Save to preset
            preset_index = len(self.preset_stations)
            if preset_index < 4:  # Maximum 4 presets
                self.preset_stations.append(self.radio.tuned_frequency)
                self.presets_changed = True
    
    def _handle_station_list_keys(self, key):
        """Handle keys in station list mode"""
        if key == 27:  # ESC key
            # Return to main screen
            self.current_state = RadioState.MAIN
        
        elif key == curses.KEY_UP:
            # Move up in list
            self.station_index = max(0, self.station_index - 1)
        
        elif key == curses.KEY_DOWN:
            # Move down in list
            self.station_index = min(len(self.station_list) - 1, self.station_index + 1)
        
        elif key == 10:  # Enter key
            # Tune to selected station
            if 0 <= self.station_index < len(self.station_list):
                freq = self.station_list[self.station_index][0]
                self.radio.tune(freq * 1e6)  # Convert MHz to Hz
                self.current_state = RadioState.MAIN
    
    def _handle_settings_keys(self, key):
        """Handle keys in settings mode"""
        if key == 27:  # ESC key
            # Return to main screen and reset adjustment flags
            self.volume_adjustment = False
            self.squelch_adjustment = False
            self.current_state = RadioState.MAIN
        
        elif key == ord('m') or key == ord('M'):
            # Change demodulation mode
            modes = list(DemodMode)
            current_index = modes.index(self.radio.demod_mode)
            next_index = (current_index + 1) % len(modes)
            self.radio.set_demod_mode(modes[next_index])
        
        elif key == ord('v') or key == ord('V'):
            # Toggle volume adjustment
            self.volume_adjustment = not self.volume_adjustment
            self.squelch_adjustment = False
        
        elif key == ord('q') or key == ord('Q'):
            # Toggle squelch adjustment
            if not self.squelch_adjustment:
                self.squelch_adjustment = True
                self.volume_adjustment = False
            else:
                # Toggle squelch on/off
                self.radio.set_squelch(not self.radio.squelch_enabled)
        
        elif key == curses.KEY_LEFT:
            # Decrease setting
            if self.volume_adjustment:
                # Decrease volume
                new_volume = max(0.0, self.radio.volume - 0.05)
                self.radio.set_volume(new_volume)
            elif self.squelch_adjustment and self.radio.squelch_enabled:
                # Decrease squelch level
                new_level = self.radio.squelch_level - 5
                self.radio.set_squelch(True, new_level)
        
        elif key == curses.KEY_RIGHT:
            # Increase setting
            if self.volume_adjustment:
                # Increase volume
                new_volume = min(1.0, self.radio.volume + 0.05)
                self.radio.set_volume(new_volume)
            elif self.squelch_adjustment and self.radio.squelch_enabled:
                # Increase squelch level
                new_level = self.radio.squelch_level + 5
                self.radio.set_squelch(True, new_level)


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description='FM Radio Command Line Interface')
    parser.add_argument('--frequency', type=float, default=100.0,
                        help='Initial frequency in MHz (default: 100.0)')
    parser.add_argument('--mode', choices=['FM', 'AM', 'NFM', 'USB', 'LSB', 'CW'],
                        default='FM', help='Demodulation mode (default: FM)')
    
    args = parser.parse_args()
    
    # Create radio
    radio = FMRadio()
    
    # Set initial parameters
    radio.tuned_frequency = args.frequency * 1e6  # Convert MHz to Hz
    radio.set_demod_mode(args.mode)
    
    # Create CLI
    cli = FMRadioCLI(radio)
    
    # Start CLI
    return 0 if cli.start() else 1


if __name__ == "__main__":
    sys.exit(main())