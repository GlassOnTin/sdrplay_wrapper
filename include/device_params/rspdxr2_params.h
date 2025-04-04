#pragma once
#include "device_registry.h"
#include "device_control.h"
#include "device_parameters.h"

namespace sdrplay {

class RSPdxR2Parameters : public DeviceParameters {
public:
    explicit RSPdxR2Parameters(DeviceControl* control = nullptr)
        : deviceControl(control) {}

    std::string getDeviceName() const override { return "RSPdxR2"; }

    void applyDefaults() override {
        if (deviceControl) {
            deviceControl->setFrequency(100.0e6);
            deviceControl->setSampleRate(2.0e6);
            deviceControl->setHDRMode(false);
            deviceControl->setBiasTEnabled(false);
        }
    }

    void setFrequency(double freq) override {
        if (deviceControl) deviceControl->setFrequency(freq);
    }

    double getFrequency() const override {
        return deviceControl ? deviceControl->getFrequency() : 0.0;
    }

    void setSampleRate(double rate) override {
        if (deviceControl) deviceControl->setSampleRate(rate);
    }

    double getSampleRate() const override {
        return deviceControl ? deviceControl->getSampleRate() : 0.0;
    }

    void setHDRMode(bool enable) {
        if (deviceControl) deviceControl->setHDRMode(enable);
    }

    void setBiasTEnabled(bool enable) {
        if (deviceControl) deviceControl->setBiasTEnabled(enable);
    }

private:
    DeviceControl* deviceControl;
};

using Rspdxr2Params = RSPdxR2Parameters;

} // namespace sdrplay
