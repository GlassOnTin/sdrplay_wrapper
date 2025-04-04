#pragma once
#include "device_control.h"
#include "sdrplay_api.h"

namespace sdrplay {

class RSP1AControl : public DeviceControl {
public:
    RSP1AControl();
    ~RSP1AControl() override;

    // Common controls implementation
    void setFrequency(double freq) override;
    double getFrequency() const override;
    void setSampleRate(double rate) override;
    double getSampleRate() const override;

    // RSP1A specific controls implementation
    void setGainReduction(int gain) override;
    void setLNAState(int state) override;

    // Unsupported RSPdxR2 controls
    void setHDRMode(bool) override { /* Not supported */ }
    void setBiasTEnabled(bool) override { /* Not supported */ }

private:
    struct RSP1AImpl;
    std::unique_ptr<RSP1AImpl> impl;
};

} // namespace sdrplay
