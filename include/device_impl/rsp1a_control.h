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
    
    // Streaming control methods
    bool initializeStreaming() override;
    bool startStreaming() override;
    bool stopStreaming() override;
    bool isStreaming() const override;
    
    // Callback registration
    void setStreamCallback(StreamCallbackHandler* handler) override;
    void setGainCallback(GainCallbackHandler* handler) override;
    void setPowerOverloadCallback(PowerOverloadCallbackHandler* handler) override;
    
    // Static callback handlers for SDRPlay API
    static void streamCallback(short* xi, short* xq, sdrplay_api_StreamCbParamsT* params, 
                              unsigned int numSamples, unsigned int reset, void* cbContext);
    static void eventCallback(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner, 
                             sdrplay_api_EventParamsT* params, void* cbContext);

private:
    struct RSP1AImpl;
    std::unique_ptr<RSP1AImpl> impl;
};

} // namespace sdrplay
