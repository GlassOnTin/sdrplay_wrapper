#!/usr/bin/env python3
import unittest
import sys
import os
import logging

# Configure logging
logging.basicConfig(level=logging.DEBUG,
                   format='%(asctime)s - %(levelname)s - %(module)s - %(funcName)s - %(message)s')
logger = logging.getLogger(__name__)

# Add the build directory to the Python path
sys.path.append(os.path.join(os.path.dirname(__file__), 'build/python'))

import sdrplay

class TestSDRplayWrapper(unittest.TestCase):
    def setUp(self):
        self.device = sdrplay.Device()
        # Open the device in setup
        self.assertTrue(self.device.open(), "Failed to open SDRPlay API")

    def tearDown(self):
        self.device.close()

    def test_api_version(self):
        """Test API version retrieval"""
        version = self.device.getApiVersion()
        logger.debug(f"API Version: {version:.5f}".rstrip('0'))
        self.assertAlmostEqual(version, 3.15, places=2)

    def test_device_enumeration(self):
        """Test device enumeration functionality"""
        devices = self.device.getAvailableDevices()
        logger.debug(f"Available Devices: {len(devices)}")

        # Accept either a DeviceInfoVector or a tuple/list containing DeviceInfo objects
        self.assertTrue(isinstance(devices, (sdrplay.DeviceInfoVector, tuple, list)),
                    f"Unexpected devices type: {type(devices)}")

        # Check that we got at least one device
        self.assertGreater(len(devices), 0, "No SDRPlay devices found")

        # Verify the first device is a DeviceInfo object
        first_device = devices[0]
        self.assertIsInstance(first_device, sdrplay.DeviceInfo,
                            f"Device is not a DeviceInfo object: {type(first_device)}")

    def test_device_info_properties(self):
        """Test device info structure properties"""
        devices = self.device.getAvailableDevices()
        num_devices = len(devices)
        logger.debug(f"Number of available devices: {num_devices}")

        if num_devices > 0:  # Check if any devices are found
            for device in devices:
                logger.debug(f"Device Serial Number: {device.serialNumber}")
                logger.debug(f"Device Hardware Version: {device.hwVersion}")
                logger.debug(f"Is Tuner A: {device.isTunerA}")
                logger.debug(f"Is Tuner B: {device.isTunerB}")
                logger.debug(f"Is RSP Duo: {device.isRSPDuo}")
                self.assertIsInstance(device.serialNumber, str)
                self.assertIsInstance(device.hwVersion, int)
                self.assertIsInstance(device.isTunerA, bool)
                self.assertIsInstance(device.isTunerB, bool)
                self.assertIsInstance(device.isRSPDuo, bool)


class TestSDRplayCallbacks(unittest.TestCase):
    def setUp(self):
        self.device = sdrplay.Device()
        self.assertTrue(self.device.open(), "Failed to open SDRPlay API")
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

def test_callbacks(self):
        """Test callback registration and handling"""
        logger.debug("Starting callback test")

        devices = self.device.getAvailableDevices()
        self.assertGreater(len(devices), 0, "No SDRPlay devices found")
        logger.debug(f"Found {len(devices)} devices")

        # Select first available device
        logger.debug("Selecting first device")
        self.assertTrue(self.device.selectDevice(devices[0]))

        # Create device parameters
        logger.debug("Getting device parameters")
        params = self.device.getDeviceParams()
        self.assertIsNotNone(params)

        # Set sample rate
        logger.debug("Setting sample rate")
        params.setSampleRate(2e6)  # 2 MHz
        self.assertTrue(params.update())

        # Set up RX channel
        logger.debug("Setting up RX channel")
        rx_params = self.device.getRxChannelParams()
        self.assertIsNotNone(rx_params)
        rx_params.setRfFrequency(100e6)  # 100 MHz
        rx_params.setBandwidth(600)  # 600 kHz
        rx_params.setGain(40, 0)  # 40 dB reduction, LNA state 0
        self.assertTrue(rx_params.update())

        # Create and register handlers
        logger.debug("Creating handlers")
        stream_handler = self.StreamHandler(self)
        gain_handler = self.GainHandler(self)
        power_handler = self.PowerHandler(self)

        logger.debug("Starting streaming")
        self.assertTrue(self.device.startStreamingWithHandlers(
            stream_handler, gain_handler, power_handler))

        # Wait for some data
        logger.debug("Waiting for data")
        import time
        time.sleep(1)

        # Stop streaming
        logger.debug("Stopping streaming")
        self.assertTrue(self.device.stopStreaming())

        # Check that we received data
        logger.debug(f"Stream data received: {self.stream_data is not None}")
        if self.stream_data is not None:
            xi, xq, numSamples = self.stream_data
            self.assertGreater(numSamples, 0)


class TestSDRplayParameters(unittest.TestCase):
    def setUp(self):
        self.device = sdrplay.Device()
        self.assertTrue(self.device.open(), "Failed to open SDRPlay API")

    def tearDown(self):
        self.device.close()

    def test_parameter_settings(self):
        """Test parameter setting and updating"""
        devices = self.device.getAvailableDevices()
        self.assertGreater(len(devices), 0, "No SDRPlay devices found")

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
