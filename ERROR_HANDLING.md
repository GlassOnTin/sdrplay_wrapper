# SDRPlay Wrapper Error Handling

This document describes the error handling system implemented in the SDRPlay wrapper library.

## Overview

The SDRPlay wrapper library implements a comprehensive exception-based error handling system. This replaces the previous approach of returning boolean success/failure values and using stderr for error messages.

The new approach offers several advantages:
- More detailed error information through specific exception types
- Consistent handling across all components of the library
- Ability to catch specific error types and handle them appropriately
- Clear error codes that can be mapped to user-friendly messages

## Exception Hierarchy

The exception hierarchy is designed to provide specific error types for different categories of errors:

- `SDRPlayException` - Base class for all exceptions in the library
  - `ApiException` - Errors from the SDRPlay API itself
  - `DeviceException` - Errors related to device operations (selection, initialization, etc.)
  - `StreamingException` - Errors related to streaming operations
  - `ParameterException` - Errors related to invalid parameters
  - `UnsupportedDeviceException` - Error when attempting to use an unsupported device type

Each exception includes:
- An error code from the `ErrorCode` enum
- A detailed message explaining the error
- A `getFullMessage()` method that combines the error description with the specific message

## Error Codes

The `ErrorCode` enum provides a consistent set of error codes across the library:

```cpp
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
```

## Using the Error Handling System

### Throwing Exceptions

When implementing new functionality, throw the appropriate exception type with a specific error code:

```cpp
if (!deviceInfo.valid) {
    throw DeviceException(ErrorCode::DEVICE_NOT_FOUND, 
                         "Device is not valid: " + deviceInfo.serialNumber);
}
```

### Catching Exceptions

Client code can catch specific exception types to handle different error categories:

```cpp
try {
    device->selectDevice(deviceInfo);
    device->startStreaming();
} catch (const DeviceException& e) {
    std::cerr << "Device error: " << e.getFullMessage() << std::endl;
} catch (const StreamingException& e) {
    std::cerr << "Streaming error: " << e.getFullMessage() << std::endl;
} catch (const SDRPlayException& e) {
    std::cerr << "SDRPlay error: " << e.getFullMessage() << std::endl;
} catch (const std::exception& e) {
    std::cerr << "Unknown error: " << e.what() << std::endl;
}
```

## Error Messages

Error messages should be clear, concise, and actionable. They should:
- Describe the error condition
- Include relevant context (e.g., device ID, parameter name)
- Suggest a resolution if possible

## Testing

A dedicated test file `test_error_handling.cpp` verifies the error handling system. It includes tests for:
- Different exception types
- Error code mapping
- Message formatting
- Common error scenarios

## Python Integration

When used through the Python binding, C++ exceptions are translated to Python exceptions. The specific exception hierarchy is preserved, allowing Python code to catch and handle specific error types.