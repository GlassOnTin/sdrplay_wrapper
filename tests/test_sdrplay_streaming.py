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

    def test_streaming_callbacks(self):
        """Test callback registration and basic streaming"""
        # Configure device directly
        self.logger.debug("Setting basic parameters")
        self.device.setSampleRate(2e6)  # 2 MHz
        self.device.setFrequency(100e6)  # 100 MHz
        
        # Note: The API has changed, we now set parameters directly on the device
        # instead of through basic_params

        # Start streaming
        self.logger.debug("Starting streaming with handlers")
        self.assertTrue(self.device.startStreamingWithHandlers(
            self.StreamHandler(self),
            self.GainHandler(self),
            self.PowerHandler(self)
        ))

        # Wait for some data
        self.logger.debug("Waiting for data...")
        time.sleep(1)

        # Stop streaming
        self.logger.debug("Stopping streaming")
        self.assertTrue(self.device.stopStreaming())

        # Verify data received
        self.assertIsNotNone(self.stream_data, "No stream data received")
        xi, xq, numSamples = self.stream_data
        self.logger.debug(f"Received {numSamples} samples")
        self.assertGreater(numSamples, 0)

if __name__ == '__main__':
    unittest.main(verbosity=2)
