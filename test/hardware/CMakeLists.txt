cmake_minimum_required(VERSION 3.16)

# ESP-IDF Hardware Testing Configuration
# This builds tests to run directly on ESP32-S3 hardware

project(ptouch_hardware_tests)

# Include ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Set component requirements for hardware testing
set(COMPONENTS
    main
    driver
    usb
    esp_http_server
    esp_wifi
    nvs_flash
    spiffs
    freertos
    esp_timer
    esp_common
    esp_system
    heap
)

# Include directories for our test framework
set(COMPONENT_ADD_INCLUDEDIRS
    "../runner"
    "../fixtures" 
    "../../components/ptouch-esp32/include"
    "../../include"
)

# Hardware test sources
set(HARDWARE_TEST_SOURCES
    "hardware_test_runner.cpp"
    "test_usb_host_hardware.cpp"
    "test_core_systems.cpp"
    "test_wifi_hardware.cpp"
    "test_integration_hardware.cpp"
)

# Add our hardware test framework
idf_component_register(
    SRCS ${HARDWARE_TEST_SOURCES}
    INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}
    REQUIRES ${COMPONENTS}
)

# Compiler flags for hardware testing
target_compile_options(${COMPONENT_LIB} PRIVATE
    -DHARDWARE_TESTING
    -DTEST_ON_DEVICE
    -Wall
    -Wextra
)

# Define test build type
if(CONFIG_HARDWARE_TEST_VERBOSE)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE TEST_VERBOSE=1)
endif()

if(CONFIG_HARDWARE_TEST_USB_HOST)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE ENABLE_USB_HOST_TESTS=1)
endif()

if(CONFIG_HARDWARE_TEST_WIFI)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE ENABLE_WIFI_TESTS=1)
endif() 