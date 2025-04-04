#!/usr/bin/env python3
from tests.test_common import *

def has_rspdxr2_device():
    """Check if RSPdxR2 device is available"""
    device = sdrplay.Device()
    try:
        devices = device.getAvailableDevices()
        return any(d.hwVer == sdrplay.RSPDXR2_HWVER for d in devices)
    except Exception as e:
        print(f"Error checking for RSPdxR2 device: {e}")
        return False


@unittest.skipUnless(has_rspdxr2_device(), "No RSPdxR2 device available")
class SDRRSPdxR2Test(SDRplayBaseTest):
    def setUp(self):
        super().setUp()
        self.device_info = self._select_rspdxr2_device()
        self.rspdxr2_params = self.device.getRspDxR2Params()
        self.assertIsNotNone(self.rspdxr2_params)

    def _select_rspdxr2_device(self):
        devices = self.device.getAvailableDevices()
        for device in devices:
            if device.hwVer == sdrplay.RSPDXR2_HWVER:
                self.assertTrue(self.device.selectDevice(device))
                return device
        self.skipTest("No RSPdxR2 device found")

    def test_basic_functionality(self):
        # Add RSPdxR2-specific tests here
        pass


if __name__ == '__main__':
    unittest.main(verbosity=2)
