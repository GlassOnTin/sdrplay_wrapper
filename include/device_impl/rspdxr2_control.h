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
    struct RSPdxR2Impl;
    std::unique_ptr<RSPdxR2Impl> impl;
};

} // namespace sdrplay
