// include/device_params/rsp1a_params.h
#ifndef SDRPLAY_RSP1A_PARAMS_H
#define SDRPLAY_RSP1A_PARAMS_H

#include <memory>

namespace sdrplay {

// Forward declare DeviceControl
class DeviceControl;

class Rsp1aParams {
public:
    explicit Rsp1aParams(DeviceControl* deviceControl);
    ~Rsp1aParams();

    void setBiasT(bool enable);
    void setRfNotch(bool enable);
    void setDabNotch(bool enable);
    bool update();

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace sdrplay

#endif // SDRPLAY_RSP1A_PARAMS_H
