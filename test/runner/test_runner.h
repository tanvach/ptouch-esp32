#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <exception>
#include <sstream>
#include <chrono>

// Test failure exception
class TestFailure : public std::exception {
private:
    std::string message;
    
public:
    TestFailure(const char* file, int line, const std::string& condition) {
        std::stringstream ss;
        ss << "Test failure at " << file << ":" << line << " - " << condition;
        message = ss.str();
    }
    
    const char* what() const noexcept override {
        return message.c_str();
    }
};

// Test registration and execution
class TestRegistry {
private:
    struct TestCase {
        std::string name;
        std::function<void()> test_func;
        std::string category;
    };
    
    std::vector<TestCase> tests;
    static TestRegistry* instance_ptr;
    
    TestRegistry() = default;
    
public:
    static TestRegistry& instance() {
        if (!instance_ptr) {
            instance_ptr = new TestRegistry();
        }
        return *instance_ptr;
    }
    
    void add_test(const std::string& name, std::function<void()> test_func, const std::string& category = "unit") {
        tests.push_back({name, test_func, category});
    }
    
    // Run all tests or filter by category
    int run_tests(const std::string& filter = "", bool verbose = false) {
        int passed = 0;
        int failed = 0;
        
        std::cout << "Running P-touch ESP32 Test Suite" << std::endl;
        std::cout << "=================================" << std::endl;
        
        auto start_time = std::chrono::steady_clock::now();
        
        for (const auto& test : tests) {
            // Apply category filter if specified
            if (!filter.empty() && test.category != filter) {
                continue;
            }
            
            if (verbose) {
                std::cout << "Running " << test.category << "::" << test.name << "... ";
            } else {
                std::cout << ".";
            }
            
            try {
                test.test_func();
                passed++;
                if (verbose) {
                    std::cout << "PASS" << std::endl;
                }
            } catch (const TestFailure& e) {
                failed++;
                if (verbose) {
                    std::cout << "FAIL" << std::endl;
                    std::cout << "  " << e.what() << std::endl;
                } else {
                    std::cout << "F";
                }
            } catch (const std::exception& e) {
                failed++;
                if (verbose) {
                    std::cout << "ERROR" << std::endl;
                    std::cout << "  Unexpected exception: " << e.what() << std::endl;
                } else {
                    std::cout << "E";
                }
            }
        }
        
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << std::endl << std::endl;
        std::cout << "Results:" << std::endl;
        std::cout << "  Passed: " << passed << std::endl;
        std::cout << "  Failed: " << failed << std::endl;
        std::cout << "  Total:  " << (passed + failed) << std::endl;
        std::cout << "  Time:   " << duration.count() << "ms" << std::endl;
        
        if (failed > 0) {
            std::cout << std::endl << "TESTS FAILED!" << std::endl;
            return 1;
        } else {
            std::cout << std::endl << "ALL TESTS PASSED!" << std::endl;
            return 0;
        }
    }
    
    size_t get_test_count() const {
        return tests.size();
    }
};

// Test macros
#define TEST(name) \
    void test_##name(); \
    namespace { \
        struct TestRegistrar_##name { \
            TestRegistrar_##name() { \
                TestRegistry::instance().add_test(#name, test_##name, "unit"); \
            } \
        }; \
        static TestRegistrar_##name registrar_##name; \
    } \
    void test_##name()

#define INTEGRATION_TEST(name) \
    void integration_test_##name(); \
    namespace { \
        struct IntegrationTestRegistrar_##name { \
            IntegrationTestRegistrar_##name() { \
                TestRegistry::instance().add_test(#name, integration_test_##name, "integration"); \
            } \
        }; \
        static IntegrationTestRegistrar_##name integration_registrar_##name; \
    } \
    void integration_test_##name()

#define PROTOCOL_TEST(name) \
    void protocol_test_##name(); \
    namespace { \
        struct ProtocolTestRegistrar_##name { \
            ProtocolTestRegistrar_##name() { \
                TestRegistry::instance().add_test(#name, protocol_test_##name, "protocol"); \
            } \
        }; \
        static ProtocolTestRegistrar_##name protocol_registrar_##name; \
    } \
    void protocol_test_##name()

// Assertion macros
#define ASSERT_EQ(expected, actual) \
    do { \
        auto exp_val = (expected); \
        auto act_val = (actual); \
        if (exp_val != act_val) { \
            std::stringstream ss; \
            ss << #expected " (" << exp_val << ") != " #actual " (" << act_val << ")"; \
            throw TestFailure(__FILE__, __LINE__, ss.str()); \
        } \
    } while(0)

#define ASSERT_NE(not_expected, actual) \
    do { \
        auto not_exp_val = (not_expected); \
        auto act_val = (actual); \
        if (not_exp_val == act_val) { \
            std::stringstream ss; \
            ss << #not_expected " (" << not_exp_val << ") == " #actual " (" << act_val << ")"; \
            throw TestFailure(__FILE__, __LINE__, ss.str()); \
        } \
    } while(0)

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            throw TestFailure(__FILE__, __LINE__, #condition " is false"); \
        } \
    } while(0)

#define ASSERT_FALSE(condition) \
    do { \
        if ((condition)) { \
            throw TestFailure(__FILE__, __LINE__, #condition " is true"); \
        } \
    } while(0)

#define ASSERT_NULL(pointer) \
    do { \
        if ((pointer) != nullptr) { \
            throw TestFailure(__FILE__, __LINE__, #pointer " is not null"); \
        } \
    } while(0)

#define ASSERT_NOT_NULL(pointer) \
    do { \
        if ((pointer) == nullptr) { \
            throw TestFailure(__FILE__, __LINE__, #pointer " is null"); \
        } \
    } while(0)

#define ASSERT_STREQ(expected, actual) \
    do { \
        std::string exp_str = (expected); \
        std::string act_str = (actual); \
        if (exp_str != act_str) { \
            std::stringstream ss; \
            ss << "String mismatch: \"" << exp_str << "\" != \"" << act_str << "\""; \
            throw TestFailure(__FILE__, __LINE__, ss.str()); \
        } \
    } while(0)

#define ASSERT_NEAR(expected, actual, tolerance) \
    do { \
        auto exp_val = (expected); \
        auto act_val = (actual); \
        auto tol_val = (tolerance); \
        auto diff = (exp_val > act_val) ? (exp_val - act_val) : (act_val - exp_val); \
        if (diff > tol_val) { \
            std::stringstream ss; \
            ss << #expected " (" << exp_val << ") and " #actual " (" << act_val << ") differ by " << diff << " > " << tol_val; \
            throw TestFailure(__FILE__, __LINE__, ss.str()); \
        } \
    } while(0)

// Utility for testing exceptions
#define ASSERT_THROWS(statement, exception_type) \
    do { \
        bool caught = false; \
        try { \
            statement; \
        } catch (const exception_type&) { \
            caught = true; \
        } catch (...) { \
            throw TestFailure(__FILE__, __LINE__, #statement " threw wrong exception type"); \
        } \
        if (!caught) { \
            throw TestFailure(__FILE__, __LINE__, #statement " did not throw " #exception_type); \
        } \
    } while(0)

#define ASSERT_NO_THROW(statement) \
    do { \
        try { \
            statement; \
        } catch (...) { \
            throw TestFailure(__FILE__, __LINE__, #statement " threw an exception"); \
        } \
    } while(0)

// Memory leak detection helper (basic)
class MemoryTracker {
private:
    size_t allocations = 0;
    size_t deallocations = 0;
    
public:
    void record_allocation() { allocations++; }
    void record_deallocation() { deallocations++; }
    
    size_t get_leaked_count() const {
        return allocations - deallocations;
    }
    
    void reset() {
        allocations = 0;
        deallocations = 0;
    }
};

// Test setup/teardown helper
class TestFixture {
public:
    virtual ~TestFixture() = default;
    virtual void setup() {}
    virtual void teardown() {}
};

#define TEST_F(fixture_class, test_name) \
    void fixture_test_##test_name(); \
    namespace { \
        struct FixtureTestRegistrar_##test_name { \
            FixtureTestRegistrar_##test_name() { \
                TestRegistry::instance().add_test(#test_name, []() { \
                    fixture_class fixture; \
                    fixture.setup(); \
                    try { \
                        fixture_test_##test_name(); \
                    } catch (...) { \
                        fixture.teardown(); \
                        throw; \
                    } \
                    fixture.teardown(); \
                }, "unit"); \
            } \
        }; \
        static FixtureTestRegistrar_##test_name fixture_registrar_##test_name; \
    } \
    void fixture_test_##test_name()

#endif // TEST_RUNNER_H 