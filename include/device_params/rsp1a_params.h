#pragma once
#include "device_registry.h"
#include "device_control.h"
#include "device_parameters.h"

namespace sdrplay {

class RSP1AParameters : public DeviceParameters {
public:
    explicit RSP1AParameters(DeviceControl* control = nullptr)
        : deviceControl(control) {}

    std::string getDeviceName() const override { return "RSP1A"; }

    void applyDefaults() override {
        if (deviceControl) {
            deviceControl->setFrequency(100.0e6);
            deviceControl->setSampleRate(2.0e6);
            deviceControl->setGainReduction(40);
            deviceControl->setLNAState(0);
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

    void setGainReduction(int gain) {
        if (deviceControl) deviceControl->setGainReduction(gain);
    }

    void setLNAState(int state) {
        if (deviceControl) deviceControl->setLNAState(state);
    }

private:
    DeviceControl* deviceControl;
};

using Rsp1aParams = RSP1AParameters;

} // namespace sdrplay
