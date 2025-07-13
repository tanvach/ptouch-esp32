#include "test_runner.h"
#include <iostream>
#include <string>
#include <cstring>

// Static instance initialization
TestRegistry* TestRegistry::instance_ptr = nullptr;

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help, -h          Show this help message" << std::endl;
    std::cout << "  --verbose, -v       Verbose output (show individual test results)" << std::endl;
    std::cout << "  --unit-only         Run only unit tests" << std::endl;
    std::cout << "  --integration-only  Run only integration tests" << std::endl;
    std::cout << "  --protocol-only     Run only protocol tests" << std::endl;
    std::cout << "  --list             List all available tests" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << "                    # Run all tests" << std::endl;
    std::cout << "  " << program_name << " --verbose          # Run all tests with verbose output" << std::endl;
    std::cout << "  " << program_name << " --unit-only        # Run only unit tests" << std::endl;
    std::cout << "  " << program_name << " --integration-only # Run only integration tests" << std::endl;
}

void list_tests() {
    auto& registry = TestRegistry::instance();
    
    std::cout << "Available tests (" << registry.get_test_count() << " total):" << std::endl;
    std::cout << std::endl;
    
    // This is a simplified version - in a real implementation, you'd want to
    // modify TestRegistry to provide access to test metadata for listing
    std::cout << "Use --unit-only, --integration-only, or --protocol-only to run specific categories." << std::endl;
    std::cout << "Run without arguments to execute all tests." << std::endl;
}

int main(int argc, char* argv[]) {
    bool verbose = false;
    std::string filter = "";
    bool list_only = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "--unit-only") == 0) {
            filter = "unit";
        } else if (strcmp(argv[i], "--integration-only") == 0) {
            filter = "integration";
        } else if (strcmp(argv[i], "--protocol-only") == 0) {
            filter = "protocol";
        } else if (strcmp(argv[i], "--list") == 0) {
            list_only = true;
        } else {
            std::cerr << "Unknown option: " << argv[i] << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (list_only) {
        list_tests();
        return 0;
    }
    
    // Print configuration
    std::cout << "P-touch ESP32 Test Suite" << std::endl;
    std::cout << "========================" << std::endl;
    std::cout << "Configuration:" << std::endl;
    std::cout << "  Verbose: " << (verbose ? "Yes" : "No") << std::endl;
    std::cout << "  Filter: " << (filter.empty() ? "All tests" : filter + " tests only") << std::endl;
    std::cout << std::endl;
    
    // Run tests
    auto& registry = TestRegistry::instance();
    
    if (registry.get_test_count() == 0) {
        std::cout << "No tests found!" << std::endl;
        std::cout << "Make sure test files are compiled and linked properly." << std::endl;
        return 1;
    }
    
    try {
        return registry.run_tests(filter, verbose);
    } catch (const std::exception& e) {
        std::cerr << "Test runner error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown test runner error occurred." << std::endl;
        return 1;
    }
} 