// include/device_control.h
#ifndef SDRPLAY_DEVICE_CONTROL_H
#define SDRPLAY_DEVICE_CONTROL_H

#include <string>
#include <memory>
#include <vector>
#include "sdrplay_api.h"

namespace sdrplay {

struct DeviceInfo {
    std::string serialNumber;
    int hwVersion;
    bool isTunerA;
    bool isTunerB;
    bool isRSPDuo;
    DeviceInfo() : hwVersion(0), isTunerA(false), isTunerB(false), isRSPDuo(false) {}
};

class DeviceControl {
public:
    DeviceControl();
    ~DeviceControl();

    bool open();
    void close();
    float getApiVersion() const;
    std::vector<DeviceInfo> getAvailableDevices();
    bool selectDevice(const DeviceInfo& device);
    bool releaseDevice();
    sdrplay_api_DeviceT* getCurrentDevice() const;
    sdrplay_api_DeviceParamsT* getDeviceParams() const;
    std::string getLastError() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace sdrplay

#endif // SDRPLAY_DEVICE_CONTROL_H
