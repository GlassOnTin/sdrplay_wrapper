#!/usr/bin/env python3
import unittest
from unittest.mock import MagicMock, patch
import numpy as np
import sys
import os

# Add the build directory to the Python path
sys.path.append(os.path.join(os.path.dirname(__file__), 'build/python'))

import sdrplay

class TestSDRplayWrapper(unittest.TestCase):
    def setUp(self):
        self.device = sdrplay.Device()

    def tearDown(self):
        self.device.close()

    def test_api_version(self):
        """Test API version retrieval"""
        version = self.device.getApiVersion()
        self.assertGreater(version, 0.0)
        self.assertLess(version, 10.0)  # Reasonable version range

    def test_device_enumeration(self):
        """Test device enumeration functionality"""
        print(self.device)
        devices = self.device.getAvailableDevices()
        #self.assertIsInstance(devices, list)
        # Note: Can't assert length as it depends on connected devices

    def test_device_info_properties(self):
        """Test device info structure properties"""
        devices = self.device.getAvailableDevices()
        if len(devices) > 0:
            info = devices[0]
            self.assertIsInstance(info.serialNumber, str)
            self.assertIsInstance(info.hwVersion, int)
            self.assertIsInstance(info.isTunerA, bool)
            self.assertIsInstance(info.isTunerB, bool)
            self.assertIsInstance(info.isRSPDuo, bool)

class TestSDRplayCallbacks(unittest.TestCase):
    def setUp(self):
        self.device = sdrplay.Device()
        self.stream_data = None
        self.gain_data = None
        self.overload_data = None

    def tearDown(self):
        self.device.close()

    class StreamHandler(sdrplay.StreamCallbackHandler):
        def __init__(self, test_instance):
            super().__init__()
            self.test_instance = test_instance

        def handleStreamData(self, xi, xq, numSamples):
            self.test_instance.stream_data = (xi, xq, numSamples)

    class GainHandler(sdrplay.GainCallbackHandler):
        def __init__(self, test_instance):
            super().__init__()
            self.test_instance = test_instance

        def handleGainChange(self, gRdB, lnaGRdB, currGain):
            self.test_instance.gain_data = (gRdB, lnaGRdB, currGain)

    class PowerHandler(sdrplay.PowerOverloadCallbackHandler):
        def __init__(self, test_instance):
            super().__init__()
            self.test_instance = test_instance

        def handlePowerOverload(self, isOverloaded):
            self.test_instance.overload_data = isOverloaded

    @unittest.skipIf(len(sdrplay.Device().getAvailableDevices()) == 0,
                    "No SDRplay devices connected")
    def test_callbacks(self):
        """Test callback registration and handling"""
        devices = self.device.getAvailableDevices()
        if not devices:
            self.skipTest("No SDRplay devices available")

        # Select first available device
        self.assertTrue(self.device.selectDevice(devices[0]))

        # Create device parameters
        params = self.device.getDeviceParams()
        self.assertIsNotNone(params)

        # Set sample rate
        params.setSampleRate(2e6)  # 2 MHz
        self.assertTrue(params.update())

        # Set up RX channel
        rx_params = self.device.getRxChannelParams()
        self.assertIsNotNone(rx_params)
        rx_params.setRfFrequency(100e6)  # 100 MHz
        rx_params.setBandwidth(600)  # 600 kHz
        rx_params.setGain(40, 0)  # 40 dB reduction, LNA state 0
        self.assertTrue(rx_params.update())

        # Start streaming with handlers
        stream_handler = self.StreamHandler(self)
        gain_handler = self.GainHandler(self)
        power_handler = self.PowerHandler(self)

        self.assertTrue(self.device.startStreamingWithHandlers(
            stream_handler, gain_handler, power_handler))

        # Wait for some data
        import time
        time.sleep(1)

        # Stop streaming
        self.assertTrue(self.device.stopStreaming())

        # Check that we received some data
        if self.stream_data is not None:
            xi, xq, numSamples = self.stream_data
            self.assertGreater(numSamples, 0)

class TestSDRplayParameters(unittest.TestCase):
    def setUp(self):
        self.device = sdrplay.Device()

    def tearDown(self):
        self.device.close()

    @unittest.skipIf(len(sdrplay.Device().getAvailableDevices()) == 0,
                    "No SDRplay devices connected")
    def test_parameter_settings(self):
        """Test parameter setting and updating"""
        devices = self.device.getAvailableDevices()
        if not devices:
            self.skipTest("No SDRplay devices available")

        # Select first available device
        self.assertTrue(self.device.selectDevice(devices[0]))

        # Test device parameters
        params = self.device.getDeviceParams()
        self.assertIsNotNone(params)

        # Test sample rate setting
        params.setSampleRate(2e6)
        params.setPpm(0.0)
        self.assertTrue(params.update())

        # Test RX parameters
        rx_params = self.device.getRxChannelParams()
        self.assertIsNotNone(rx_params)

        # Test frequency setting
        rx_params.setRfFrequency(100e6)
        self.assertTrue(rx_params.update())

        # Test bandwidth setting
        rx_params.setBandwidth(600)
        self.assertTrue(rx_params.update())

        # Test IF setting
        rx_params.setIFType(0)  # Zero IF
        self.assertTrue(rx_params.update())

        # Test gain setting
        rx_params.setGain(40, 0)
        self.assertTrue(rx_params.update())

        # Test AGC setting
        rx_params.setAgcControl(True, -30)
        self.assertTrue(rx_params.update())

if __name__ == '__main__':
    unittest.main()
