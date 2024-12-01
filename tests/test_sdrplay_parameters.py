#!/usr/bin/env python3
from test_common import *

class SDRplayBasicParametersTest(SDRplayBaseTest):
    """Tests for parameters common to all devices"""

    def setUp(self):
        super().setUp()
        self.device_info = self._select_first_available_device()
        self.basic_params = self.device.getBasicParams()
        self.assertIsNotNone(self.basic_params)

    def test_sample_rate(self):
        self.basic_params.setSampleRate(2e6)
        self.assertTrue(self.basic_params.update())

    def test_frequency(self):
        self.basic_params.setRfFrequency(100e6)
        self.assertTrue(self.basic_params.update())

if __name__ == '__main__':
    unittest.main(verbosity=2)
