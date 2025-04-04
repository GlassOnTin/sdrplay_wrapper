#pragma once
#include <stdexcept>
#include <string>
#include <map>

namespace sdrplay {

/**
 * Enum defining error codes for the SDRPlay wrapper library
 */
enum class ErrorCode {
    // General errors
    SUCCESS = 0,
    UNKNOWN_ERROR,
    API_ERROR,
    
    // Device errors
    DEVICE_NOT_FOUND,
    DEVICE_NOT_SUPPORTED,
    DEVICE_ALREADY_SELECTED,
    DEVICE_NOT_SELECTED,
    DEVICE_NOT_INITIALIZED,
    
    // Streaming errors
    STREAMING_ERROR,
    STREAMING_ALREADY_ACTIVE,
    STREAMING_NOT_ACTIVE,
    
    // Parameter errors
    INVALID_PARAMETER,
    PARAMETER_OUT_OF_RANGE,
    
    // Registry errors
    UNSUPPORTED_DEVICE
};

/**
 * Base exception class for SDRPlay wrapper
 */
class SDRPlayException : public std::runtime_error {
public:
    explicit SDRPlayException(ErrorCode code, const std::string& message);
    virtual ~SDRPlayException() = default;
    
    ErrorCode getErrorCode() const;
    std::string getFullMessage() const;
    
private:
    ErrorCode errorCode;
};

/**
 * Exception thrown when an API error occurs
 */
class ApiException : public SDRPlayException {
public:
    explicit ApiException(const std::string& apiError);
};

/**
 * Exception thrown when a device error occurs
 */
class DeviceException : public SDRPlayException {
public:
    explicit DeviceException(ErrorCode code, const std::string& message);
};

/**
 * Exception thrown when a streaming error occurs
 */
class StreamingException : public SDRPlayException {
public:
    explicit StreamingException(ErrorCode code, const std::string& message);
};

/**
 * Exception thrown when an invalid parameter is provided
 */
class ParameterException : public SDRPlayException {
public:
    explicit ParameterException(ErrorCode code, const std::string& message);
};

/**
 * Exception thrown when an unsupported device is encountered
 */
class UnsupportedDeviceException : public SDRPlayException {
public:
    explicit UnsupportedDeviceException(const std::string& deviceType);
};

/**
 * Get a human-readable description for an error code
 */
std::string getErrorDescription(ErrorCode code);

} // namespace sdrplay