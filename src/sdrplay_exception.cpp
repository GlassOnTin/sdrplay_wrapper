#include "sdrplay_exception.h"
#include <unordered_map>

namespace sdrplay {

namespace {
    // Map of error codes to human-readable descriptions
    const std::unordered_map<ErrorCode, std::string> ERROR_DESCRIPTIONS = {
        {ErrorCode::SUCCESS, "Success"},
        {ErrorCode::UNKNOWN_ERROR, "Unknown error"},
        {ErrorCode::API_ERROR, "SDRPlay API error"},
        
        {ErrorCode::DEVICE_NOT_FOUND, "Device not found"},
        {ErrorCode::DEVICE_NOT_SUPPORTED, "Device not supported"},
        {ErrorCode::DEVICE_ALREADY_SELECTED, "Device already selected"},
        {ErrorCode::DEVICE_NOT_SELECTED, "No device selected"},
        {ErrorCode::DEVICE_NOT_INITIALIZED, "Device not initialized"},
        
        {ErrorCode::STREAMING_ERROR, "Streaming error"},
        {ErrorCode::STREAMING_ALREADY_ACTIVE, "Streaming already active"},
        {ErrorCode::STREAMING_NOT_ACTIVE, "Streaming not active"},
        
        {ErrorCode::INVALID_PARAMETER, "Invalid parameter"},
        {ErrorCode::PARAMETER_OUT_OF_RANGE, "Parameter out of range"},
        
        {ErrorCode::UNSUPPORTED_DEVICE, "Unsupported device hardware version"}
    };
}

SDRPlayException::SDRPlayException(ErrorCode code, const std::string& message)
    : std::runtime_error(message), errorCode(code) {}

ErrorCode SDRPlayException::getErrorCode() const {
    return errorCode;
}

std::string SDRPlayException::getFullMessage() const {
    return getErrorDescription(errorCode) + ": " + what();
}

ApiException::ApiException(const std::string& apiError)
    : SDRPlayException(ErrorCode::API_ERROR, apiError) {}

DeviceException::DeviceException(ErrorCode code, const std::string& message)
    : SDRPlayException(code, message) {}

StreamingException::StreamingException(ErrorCode code, const std::string& message)
    : SDRPlayException(code, message) {}

ParameterException::ParameterException(ErrorCode code, const std::string& message)
    : SDRPlayException(code, message) {}

UnsupportedDeviceException::UnsupportedDeviceException(const std::string& deviceType)
    : SDRPlayException(ErrorCode::UNSUPPORTED_DEVICE, 
                      "Unsupported device hardware version: " + deviceType) {}

std::string getErrorDescription(ErrorCode code) {
    auto it = ERROR_DESCRIPTIONS.find(code);
    if (it != ERROR_DESCRIPTIONS.end()) {
        return it->second;
    }
    return "Unknown error code";
}

} // namespace sdrplay