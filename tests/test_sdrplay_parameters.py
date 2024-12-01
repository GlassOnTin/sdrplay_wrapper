#!/usr/bin/env python3
import unittest
import sys
import os
import logging

# Configure logging
logging.basicConfig(level=logging.DEBUG,
                   format='%(asctime)s - %(levelname)s - %(module)s - %(funcName)s - %(message)s')
logger = logging.getLogger(__name__)

import sdrplay

class TestSDRplayParameters(unittest.TestCase):
    def setUp(self):
        logger.debug("Creating Device instance")
        self.device = sdrplay.Device()
        logger.debug("Opening device")
        self.assertTrue(self.device.open(), "Failed to open SDRPlay API")
        logger.debug("Device opened successfully")

    def tearDown(self):
        logger.debug("Closing device")
        self.device.close()
        logger.debug("Device closed")

    def test_parameter_settings(self):
        """Test parameter setting and updating step by step"""
        try:
            # 1. Get devices
            logger.debug("Getting available devices")
            devices = self.device.getAvailableDevices()
            num_devices = len(devices)
            logger.debug(f"Found {num_devices} devices")
            self.assertGreater(num_devices, 0, "No SDRPlay devices found")

            # 2. Select device
            logger.debug("Selecting first device")
            device = devices[0]
            logger.debug(f"Device info - Serial: {device.serialNumber}, HW Ver: {device.hwVersion}")
            result = self.device.selectDevice(device)
            logger.debug(f"selectDevice result: {result}")
            self.assertTrue(result, "Failed to select device")

            # 3. Get device parameters
            logger.debug("Getting device parameters")
            params = self.device.getDeviceParams()
            logger.debug(f"Device params object: {params}")
            self.assertIsNotNone(params, "Failed to get device parameters")

            # 4. Set and update sample rate
            logger.debug("Setting sample rate")
            params.setSampleRate(2e6)
            logger.debug("Setting PPM")
            params.setPpm(0.0)
            logger.debug("Updating device params")
            result = params.update()
            logger.debug(f"Update result: {result}")
            self.assertTrue(result, "Failed to update device parameters")

            # 5. Get RX parameters
            logger.debug("Getting RX parameters")
            rx_params = self.device.getRxChannelParams()
            logger.debug(f"RX params object: {rx_params}")
            self.assertIsNotNone(rx_params, "Failed to get RX parameters")

            # 6. Set and update frequency
            logger.debug("Setting RF frequency")
            rx_params.setRfFrequency(100e6)
            logger.debug("Updating RX params")
            result = rx_params.update()
            logger.debug(f"Update result: {result}")
            self.assertTrue(result, "Failed to update RF frequency")

            # 7. Set and update bandwidth
            logger.debug("Setting bandwidth")
            rx_params.setBandwidth(600)
            logger.debug("Updating RX params")
            result = rx_params.update()
            logger.debug(f"Update result: {result}")
            self.assertTrue(result, "Failed to update bandwidth")

            # 8. Set and update IF
            logger.debug("Setting IF type")
            rx_params.setIFType(0)
            logger.debug("Updating RX params")
            result = rx_params.update()
            logger.debug(f"Update result: {result}")
            self.assertTrue(result, "Failed to update IF type")

            # 9. Set and update gain
            logger.debug("Setting gain")
            rx_params.setGain(40, 0)
            logger.debug("Updating RX params")
            result = rx_params.update()
            logger.debug(f"Update result: {result}")
            self.assertTrue(result, "Failed to update gain")

            # 10. Set and update AGC
            logger.debug("Setting AGC")
            rx_params.setAgcControl(True, -30)
            logger.debug("Updating RX params")
            result = rx_params.update()
            logger.debug(f"Update result: {result}")
            self.assertTrue(result, "Failed to update AGC")

            logger.debug("All parameter tests completed successfully")

        except Exception as e:
            logger.error(f"Exception during test: {str(e)}", exc_info=True)
            raise

if __name__ == '__main__':
    unittest.main(verbosity=2)
