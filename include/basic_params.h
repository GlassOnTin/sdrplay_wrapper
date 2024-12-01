// include/basic_params.h
#ifndef SDRPLAY_BASIC_PARAMS_H
#define SDRPLAY_BASIC_PARAMS_H

#include <memory>

namespace sdrplay {

// Forward declare DeviceControl
class DeviceControl;

class BasicParams {
public:
    explicit BasicParams(DeviceControl* deviceControl);
    ~BasicParams();

    void setSampleRate(double sampleRateHz);
    void setRfFrequency(double frequencyHz);
    void setBandwidth(int bandwidthKHz);
    void setIfType(int ifkHz);
    void setGain(int gainReduction, int lnaState);
    bool update();

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace sdrplay

#endif // SDRPLAY_BASIC_PARAMS_H
