#pragma once
#include <string>

namespace sdrplay {

// Maps to sdrplay_api_TunerSelectT
enum class TunerSelect {
    Neither = 0,
    A = 1,
    B = 2,
    Both = 3
};

// Maps to sdrplay_api_RspDuoModeT
enum class RspDuoMode {
    Unknown = 0,
    Single_Tuner = 1,
    Dual_Tuner = 2,
    Master = 4,
    Slave = 8
};

// Models sdrplay_api_DeviceT
struct DeviceInfo {
    std::string serialNumber;     // Maps to SerNo[SDRPLAY_MAX_SER_NO_LEN]
    unsigned char hwVer;          // Hardware version of the device
    TunerSelect tuner;           // Available/selected tuners
    RspDuoMode rspDuoMode;      // For RSPduo devices - available/selected mode
    bool valid;                  // Indicates device is ready to use
    double rspDuoSampleFreq;    // Sample rate for RSPduo slaves/masters
    void* dev;                  // Handle for subsequent API calls

    // Constructor with defaults
    DeviceInfo() :
        hwVer(0),
        tuner(TunerSelect::Neither),
        rspDuoMode(RspDuoMode::Unknown),
        valid(false),
        rspDuoSampleFreq(0.0),
        dev(nullptr)
    {}
};

// Hardware version constants from API
constexpr unsigned char RSP1_HWVER = 1;
constexpr unsigned char RSP1A_HWVER = 255;
constexpr unsigned char RSP2_HWVER = 2;
constexpr unsigned char RSPDUO_HWVER = 3;
constexpr unsigned char RSPDX_HWVER = 4;
constexpr unsigned char RSP1B_HWVER = 6;
constexpr unsigned char RSPDXR2_HWVER = 7;

} // namespace sdrplay
