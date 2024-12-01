#include "sdrplay_api.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cstring>

int main() {
    // Try getting API version before initialization
    float apiVersion;
    sdrplay_api_ErrT err = sdrplay_api_ApiVersion(&apiVersion);
    std::cout << "Initial API Version call result: " << sdrplay_api_GetErrorString(err) << std::endl;

    // Open the API first
    err = sdrplay_api_Open();
    if (err != sdrplay_api_Success) {
        std::cerr << "sdrplay_api_Open failed: " << sdrplay_api_GetErrorString(err) << std::endl;
        return 1;
    }

    std::cout << "API opened successfully, waiting for initialization..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Try getting version again after initialization
    err = sdrplay_api_ApiVersion(&apiVersion);
    std::cout << "API Version call after init result: " << sdrplay_api_GetErrorString(err) << std::endl;
    if (err == sdrplay_api_Success) {
        std::cout << "Runtime API Version: " << std::fixed << std::setprecision(2) << apiVersion << std::endl;
    } else {
        std::cout << "Using compile-time API Version: " << SDRPLAY_API_VERSION << std::endl;
    }

    // Now enumerate devices
    sdrplay_api_DeviceT devices[SDRPLAY_MAX_DEVICES];
    unsigned int numDevs;

    sdrplay_api_LockDeviceApi();
    err = sdrplay_api_GetDevices(devices, &numDevs, SDRPLAY_MAX_DEVICES);
    if (err != sdrplay_api_Success) {
        std::cerr << "GetDevices failed: " << sdrplay_api_GetErrorString(err) << std::endl;
        sdrplay_api_UnlockDeviceApi();
        sdrplay_api_Close();
        return 1;
    }

    std::cout << "sdrplay_api_GetDevices result: " << sdrplay_api_GetErrorString(err) << std::endl;
    std::cout << "Number of devices found: " << numDevs << std::endl;

    if (numDevs > 0) {
        for (unsigned int i = 0; i < numDevs; i++) {
            std::cout << "Device " << i + 1 << ":" << std::endl;
            std::cout << "  Serial Number: " << devices[i].SerNo << std::endl;
            std::cout << "  Hardware Ver: " << (int)devices[i].hwVer << std::endl;
            std::cout << "  Tuner(s): "
                      << ((devices[i].tuner & sdrplay_api_Tuner_A) ? "A " : "")
                      << ((devices[i].tuner & sdrplay_api_Tuner_B) ? "B" : "") << std::endl;
        }
    }

    sdrplay_api_UnlockDeviceApi();
    sdrplay_api_Close();
    return 0;
}
