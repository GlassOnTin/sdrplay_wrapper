#!/usr/bin/env python3
import time
from tests.test_common import *

class SDRplayStreamingTest(SDRplayBaseTest):
    """Tests for streaming functionality common to all devices"""

    def setUp(self):
        super().setUp()  # Handle device creation/opening
        self.stream_data = None
        self.gain_data = None
        self.overload_data = None
        # We already have self.device_info from SDRplayBaseTest

    class StreamHandler(sdrplay.StreamCallbackHandler):
        def __init__(self, test_instance):
            sdrplay.StreamCallbackHandler.__init__(self)
            self.test_instance = test_instance

        def handleStreamData(self, xi, xq, numSamples):
            self.test_instance.logger.debug(f"Stream data received: {numSamples} samples")
            self.test_instance.stream_data = (xi, xq, numSamples)

    class GainHandler(sdrplay.GainCallbackHandler):
        def __init__(self, test_instance):
            sdrplay.GainCallbackHandler.__init__(self)
            self.test_instance = test_instance

        def handleGainChange(self, gRdB, lnaGRdB, currGain):
            self.test_instance.logger.debug(
                f"Gain change: gRdB={gRdB}, lnaGRdB={lnaGRdB}, currGain={currGain}")
            self.test_instance.gain_data = (gRdB, lnaGRdB, currGain)

    class PowerHandler(sdrplay.PowerOverloadCallbackHandler):
        def __init__(self, test_instance):
            sdrplay.PowerOverloadCallbackHandler.__init__(self)
            self.test_instance = test_instance

        def handlePowerOverload(self, isOverloaded):
            self.test_instance.logger.debug(f"Power overload: {isOverloaded}")
            self.test_instance.overload_data = isOverloaded

    @unittest.skip("Stream API has changed, test needs updating")
    def test_streaming_callbacks(self):
        """Test callback registration and basic streaming"""
        self.logger.info("Streaming test skipped - API has changed")
        # This test needs to be updated for the new API
        # The streaming callbacks have changed

if __name__ == '__main__':
    unittest.main(verbosity=2)
