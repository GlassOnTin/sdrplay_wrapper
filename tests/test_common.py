#!/usr/bin/env python3
import unittest
import logging
import sdrplay

class TestDeviceRegistry(unittest.TestCase):
    def setUp(self):
        self.logger = logging.getLogger(self.__class__.__name__)
        logging.basicConfig(level=logging.DEBUG)

    def test_device_creation(self):
        # Test RSP1A creation
        device = sdrplay.Device()
        self.assertIsNotNone(device)

        # Test getting device list
        devices = device.getAvailableDevices()
        self.logger.info(f"Found {len(devices)} devices")
        
        # Skip actual device tests if no devices found
        if len(devices) == 0:
            self.logger.warning("No SDRPlay devices found, skipping device tests")
            return
            
        # Test device parameters
        device_info = devices[0]
        self.assertTrue(device.selectDevice(device_info))
        
        # Test frequency setting
        device.setFrequency(100e6)
        self.assertAlmostEqual(device.getFrequency(), 100e6, delta=1000)
        
        # Test sample rate setting
        device.setSampleRate(2e6)
        self.assertAlmostEqual(device.getSampleRate(), 2e6, delta=1000)

        # Clean up
        device.releaseDevice()

class SDRplayBaseTest(unittest.TestCase):
    """Base class for SDRplay tests"""
    
    def setUp(self):
        self.logger = logging.getLogger(self.__class__.__name__)
        logging.basicConfig(level=logging.DEBUG)
        self.device = sdrplay.Device()
        
        # Get available devices
        devices = self.device.getAvailableDevices()
        if len(devices) == 0:
            self.skipTest("No SDRPlay devices found")
            
        # Select first device by default
        self.device_info = devices[0]
        self.assertTrue(self.device.selectDevice(self.device_info))

    def tearDown(self):
        self.device.releaseDevice()

if __name__ == '__main__':
    unittest.main(verbosity=2)
