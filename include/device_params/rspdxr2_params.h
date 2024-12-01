// include/device_params/rsp1a_params.h
#ifndef SDRPLAY_RSPDXR2_PARAMS_H
#define SDRPLAY_RSPDXR2_PARAMS_H

#include <memory>

namespace sdrplay {

// Forward declare DeviceControl
class DeviceControl;

class RspDxR2Params {
public:
    explicit RspDxR2Params(DeviceControl* deviceControl);
    ~RspDxR2Params();

    // TODO: Add RSPdxR2-specific methods

    bool update();
private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace sdrplay

#endif // SDRPLAY_RSPDXR2_PARAMS_H
