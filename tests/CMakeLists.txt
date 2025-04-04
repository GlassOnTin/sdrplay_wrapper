# Test configuration
enable_testing()

# Build test_sdrplay_api
add_executable(test_sdrplay_api tests/test_sdrplay_api.cpp)
target_link_libraries(test_sdrplay_api PRIVATE sdrplay_wrapper)
add_test(NAME test_sdrplay_api COMMAND test_sdrplay_api)

# Build test_device_registry with testing flag
add_executable(test_device_registry tests/test_device_registry.cpp)
target_link_libraries(test_device_registry PRIVATE sdrplay_wrapper)
target_compile_definitions(test_device_registry PRIVATE SDRPLAY_TESTING)
add_test(NAME test_device_registry COMMAND test_device_registry)

# Build test_error_handling with testing flag
add_executable(test_error_handling tests/test_error_handling.cpp)
target_link_libraries(test_error_handling PRIVATE sdrplay_wrapper)
target_compile_definitions(test_error_handling PRIVATE SDRPLAY_TESTING)
add_test(NAME test_error_handling COMMAND test_error_handling)
