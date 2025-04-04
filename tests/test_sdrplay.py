#!/usr/bin/env python3
"""
Main test runner for sdrplay wrapper tests
This file addresses issue #1: Tests fail
"""

import unittest
import logging
import sys
import os

# Set up logging
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger("SDRPlayTests")

# Load all tests
if __name__ == '__main__':
    # Add the parent directory to the path
    sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    
    # Initialize sdrplay module
    try:
        import sdrplay
        # Initialize device registry explicitly
        sdrplay.initializeDeviceRegistry()
        logger.info("SDRPlay module loaded successfully and device registry initialized")
    except ImportError as e:
        logger.error(f"Failed to import SDRPlay module: {e}")
        sys.exit(1)
    
    # Discover and run all tests
    loader = unittest.TestLoader()
    start_dir = os.path.dirname(os.path.abspath(__file__))
    suite = loader.discover(start_dir, pattern="test_sdrplay_*.py")
    
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    sys.exit(0 if result.wasSuccessful() else 1)