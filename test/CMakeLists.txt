cmake_minimum_required(VERSION 3.16)

project(ptouch_tests CXX)

# C++17 required for std::filesystem and other modern features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Debug build by default for testing
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Compiler flags for testing
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

# Enable code coverage if available
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    option(ENABLE_COVERAGE "Enable code coverage" OFF)
    if(ENABLE_COVERAGE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    endif()
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/runner
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_CURRENT_SOURCE_DIR}/fixtures
    ${CMAKE_CURRENT_SOURCE_DIR}/../components/ptouch-esp32/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Test runner framework
set(TEST_RUNNER_SOURCES
    runner/main.cpp
    runner/test_runner.h
)

# Mock implementations
set(MOCK_SOURCES
    mocks/hardware_abstraction.h
    mocks/mock_usb_host.h
    # Add other mock sources as they're created
)

# Test fixtures and utilities
set(FIXTURE_SOURCES
    fixtures/test_data.h
    fixtures/mock_responses.h
    fixtures/test_helpers.h
)

# Unit tests
set(UNIT_TEST_SOURCES
    unit/test_protocol_parsing.cpp
    unit/test_image_processing.cpp
    unit/test_printer_state.cpp
    unit/test_web_endpoints.cpp
    unit/test_usb_communication.cpp
)

# Integration tests
set(INTEGRATION_TEST_SOURCES
    integration/test_print_workflow.cpp
    integration/test_error_handling.cpp
    integration/test_connection_lifecycle.cpp
    integration/test_concurrent_operations.cpp
)

# Protocol tests
set(PROTOCOL_TEST_SOURCES
    protocol/test_command_generation.cpp
    protocol/test_response_parsing.cpp
    protocol/test_printer_models.cpp
)

# Production code that we're testing (simplified versions that compile without ESP-IDF)
# We'll need to create test-compatible versions of these
set(PRODUCTION_CODE_SOURCES
    # These would be simplified/adapted versions of the production code
    # For now, we'll create stub implementations
)

# All test sources
set(ALL_TEST_SOURCES
    ${TEST_RUNNER_SOURCES}
    ${MOCK_SOURCES}
    ${FIXTURE_SOURCES}
    ${UNIT_TEST_SOURCES}
    ${INTEGRATION_TEST_SOURCES}
    ${PROTOCOL_TEST_SOURCES}
)

# Main test executable
add_executable(ptouch_tests
    ${ALL_TEST_SOURCES}
)

# Link libraries (if needed)
# target_link_libraries(ptouch_tests pthread) # If using threading

# Custom targets for different test categories
add_custom_target(test-unit
    COMMAND ptouch_tests --unit-only
    DEPENDS ptouch_tests
    COMMENT "Running unit tests only"
)

add_custom_target(test-integration
    COMMAND ptouch_tests --integration-only
    DEPENDS ptouch_tests
    COMMENT "Running integration tests only"
)

add_custom_target(test-protocol
    COMMAND ptouch_tests --protocol-only
    DEPENDS ptouch_tests
    COMMENT "Running protocol tests only"
)

add_custom_target(test-verbose
    COMMAND ptouch_tests --verbose
    DEPENDS ptouch_tests
    COMMENT "Running all tests with verbose output"
)

# Code coverage target
if(ENABLE_COVERAGE)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            # Cleanup lcov
            COMMAND ${LCOV_PATH} --directory . --zerocounters
            
            # Run tests
            COMMAND ptouch_tests
            
            # Capture coverage info
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
            
            # Filter out system and test files
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '*/test/*' --output-file coverage.info
            
            # Generate HTML report
            COMMAND ${GENHTML_PATH} coverage.info --output-directory coverage
            
            DEPENDS ptouch_tests
            COMMENT "Generating code coverage report"
        )
    else()
        message(WARNING "lcov or genhtml not found, coverage target not available")
    endif()
endif()

# Install target (optional)
install(TARGETS ptouch_tests
    RUNTIME DESTINATION bin
)

# Test discovery for IDEs
enable_testing()
add_test(NAME all_tests COMMAND ptouch_tests)
add_test(NAME unit_tests COMMAND ptouch_tests --unit-only)
add_test(NAME integration_tests COMMAND ptouch_tests --integration-only)
add_test(NAME protocol_tests COMMAND ptouch_tests --protocol-only)

# Print configuration summary
message(STATUS "")
message(STATUS "P-touch ESP32 Test Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  Coverage: ${ENABLE_COVERAGE}")
message(STATUS "")
message(STATUS "Available targets:")
message(STATUS "  ptouch_tests    - Build test executable")
message(STATUS "  test-unit       - Run unit tests only")
message(STATUS "  test-integration - Run integration tests only")
message(STATUS "  test-protocol   - Run protocol tests only")
message(STATUS "  test-verbose    - Run all tests with verbose output")
if(ENABLE_COVERAGE)
    message(STATUS "  coverage        - Generate code coverage report")
endif()
message(STATUS "") 