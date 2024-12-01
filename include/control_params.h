// include/control_params.h
#ifndef SDRPLAY_CONTROL_PARAMS_H
#define SDRPLAY_CONTROL_PARAMS_H

#include <memory>

namespace sdrplay {

// Forward declare DeviceControl
class DeviceControl;

class ControlParams {
public:
    explicit ControlParams(DeviceControl* deviceControl);
    ~ControlParams();

    void setAgcControl(bool enable, int setPoint = -60);
    void setDcOffset(bool dcEnable, bool iqEnable);
    void setDecimation(bool enable, unsigned char decimationFactor, bool wideBandSignal);
    bool update();

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace sdrplay

#endif // SDRPLAY_CONTROL_PARAMS_H
