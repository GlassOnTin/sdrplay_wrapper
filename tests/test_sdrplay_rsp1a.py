#!/usr/bin/env python3
from tests.test_common import *

def has_rsp1a_device():
    """Check if RSP1A device is available"""
    device = sdrplay.Device()
    try:
        devices = device.getAvailableDevices()
        return any(d.hwVer == sdrplay.RSP1A_HWVER for d in devices)
    except Exception as e:
        print(f"Error checking for RSP1A device: {e}")
        return False

@unittest.skipUnless(has_rsp1a_device(), "No RSP1A device available")
class SDRRSP1ATest(SDRplayBaseTest):
    """Tests specific to RSP1A devices"""

    def setUp(self):
        super().setUp()
        if not has_rsp1a_device():
            self.skipTest("No RSP1A device available")

        self.device_info = self._select_rsp1a_device()
        self.rsp1a_params = self.device.getRsp1aParams()
        self.assertIsNotNone(self.rsp1a_params)

    def _select_rsp1a_device(self):
        devices = self.device.getAvailableDevices()
        for device in devices:
            if device.hwVersion == sdrplay.SDRPLAY_RSP1A_ID:
                self.assertTrue(self.device.selectDevice(device))
                return device
        self.skipTest("No RSP1A device found")

    def test_bias_t(self):
        self.rsp1a_params.setBiasT(True)
        self.assertTrue(self.rsp1a_params.update())

if __name__ == '__main__':
    unittest.main(verbosity=2)
