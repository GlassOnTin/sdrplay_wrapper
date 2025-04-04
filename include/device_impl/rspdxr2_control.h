#pragma once
#include "device_control.h"
#include "sdrplay_api.h"

namespace sdrplay {

class RSPdxR2Control : public DeviceControl {
public:
    RSPdxR2Control();
    ~RSPdxR2Control() override;

    // Common controls implementation
    void setFrequency(double freq) override;
    double getFrequency() const override;
    void setSampleRate(double rate) override;
    double getSampleRate() const override;

    // Unsupported RSP1A controls
    void setGainReduction(int) override { /* Not supported */ }
    void setLNAState(int) override { /* Not supported */ }

    // RSPdxR2 specific controls implementation
    void setHDRMode(bool enable) override;
    void setBiasTEnabled(bool enable) override;

private:
    struct RSPdxR2Impl;
    std::unique_ptr<RSPdxR2Impl> impl;
};

} // namespace sdrplay
