#include <iostream>
#include <sdrplay_api.h>

int main() {
    // Open API
    std::cout << "Opening SDRplay API..." << std::endl;
    sdrplay_api_ErrT err = sdrplay_api_Open();
    if (err != sdrplay_api_Success) {
        std::cerr << "Failed to open API: " << sdrplay_api_GetErrorString(err) << std::endl;
        return 1;
    }
    
    // Get API version
    float apiVer;
    err = sdrplay_api_ApiVersion(&apiVer);
    if (err != sdrplay_api_Success) {
        std::cerr << "Failed to get API version: " << sdrplay_api_GetErrorString(err) << std::endl;
        sdrplay_api_Close();
        return 1;
    }
    std::cout << "API Version: " << apiVer << std::endl;
    
    // Get device list
    sdrplay_api_DeviceT devices[SDRPLAY_MAX_DEVICES];
    unsigned int numDevs = 0;
    
    std::cout << "Getting device list..." << std::endl;
    sdrplay_api_LockDeviceApi();
    err = sdrplay_api_GetDevices(devices, &numDevs, SDRPLAY_MAX_DEVICES);
    
    if (err != sdrplay_api_Success) {
        std::cerr << "Failed to get devices: " << sdrplay_api_GetErrorString(err) << std::endl;
        sdrplay_api_UnlockDeviceApi();
        sdrplay_api_Close();
        return 1;
    }
    
    std::cout << "Found " << numDevs << " device(s)" << std::endl;
    for (unsigned int i = 0; i < numDevs; i++) {
        std::cout << "Device " << i+1 << ":" << std::endl;
        std::cout << "  Serial Number: " << devices[i].SerNo << std::endl;
        std::cout << "  Hardware Ver: " << static_cast<int>(devices[i].hwVer) << std::endl;
    }
    
    sdrplay_api_UnlockDeviceApi();
    
    // Done
    std::cout << "Closing API..." << std::endl;
    sdrplay_api_Close();
    std::cout << "Done!" << std::endl;
    
    return 0;
}