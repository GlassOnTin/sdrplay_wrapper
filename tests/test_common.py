#!/usr/bin/env python3
import unittest
import sys
import os
import logging
import time
import sdrplay

# Configure logging
logging.basicConfig(
    level=logging.DEBUG,
    format='%(asctime)s - %(levelname)s - %(module)s - %(funcName)s - %(message)s'
)

# Base test class
class SDRplayBaseTest(unittest.TestCase):
    def setUp(self):
        self.logger = logging.getLogger(self.__class__.__name__)
        self.logger.debug("Creating Device instance")
        self.device = sdrplay.Device()
        self.logger.debug("Opening device")
        self.assertTrue(self.device.open(), "Failed to open SDRPlay API")
        self.logger.debug("Device opened successfully")

    def tearDown(self):
        self.logger.debug("Closing device")
        self.device.close()
        self.logger.debug("Device closed")

    def _select_first_available_device(self):
        """Helper to select first available device and return its info"""
        devices = self.device.getAvailableDevices()
        self.assertGreater(len(devices), 0, "No SDRPlay devices found")
        device = devices[0]
        self.assertTrue(self.device.selectDevice(device))
        return device
