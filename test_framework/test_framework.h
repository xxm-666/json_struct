#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>

namespace TestFramework {

class TestResult {
public:
    TestResult() : passed_(0), failed_(0) {}
    
    void addPass() { passed_++; }
    void addPass(int count) { passed_ += count; }
    void addFail(const std::string& message) {
        failed_++;
        failures_.push_back(message);
    }
    
    int getPassed() const { return passed_; }
    int getFailed() const { return failed_; }
    int getTotal() const { return passed_ + failed_; }
    
    const std::vector<std::string>& getFailures() const { return failures_; }
    
    bool isSuccess() const { return failed_ == 0; }

private:
    int passed_;
    int failed_;
    std::vector<std::string> failures_;
};

class TestCase {
public:
    TestCase(const std::string& name, std::function<void(TestResult&)> func)
        : name_(name), func_(func) {}
    
    void run(TestResult& result) {
        std::cout << "Running " << name_ << "..." << std::endl;
        func_(result);
    }
    
    const std::string& getName() const { return name_; }

private:
    std::string name_;
    std::function<void(TestResult&)> func_;
};

class TestSuite {
public:
    static TestSuite& instance() {
        static TestSuite suite;
        return suite;
    }
    
    void addTest(const std::string& name, std::function<void(TestResult&)> func) {
        tests_.emplace_back(name, func);
    }
    
    int runAll() {
        TestResult totalResult;
        std::cout << "=== Running Test Suite ===" << std::endl;
        
        for (auto& test : tests_) {
            TestResult testResult;
            test.run(testResult);
            
            if (testResult.isSuccess()) {
                std::cout << "[PASS] " << test.getName() << std::endl;
            } else {
                std::cout << "[FAIL] " << test.getName() << " (" 
                         << testResult.getFailed() << " failures)" << std::endl;
                for (const auto& failure : testResult.getFailures()) {
                    std::cout << "  - " << failure << std::endl;
                }
            }
            
            totalResult.addPass(testResult.getPassed());
            for (const auto& failure : testResult.getFailures()) {
                totalResult.addFail(failure);
            }
        }
        
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Total: " << totalResult.getTotal() << std::endl;
        std::cout << "Passed: " << totalResult.getPassed() << std::endl;
        std::cout << "Failed: " << totalResult.getFailed() << std::endl;
        
        if (totalResult.isSuccess()) {
            std::cout << "ALL TESTS PASSED!" << std::endl;
            return 0;
        } else {
            std::cout << "SOME TESTS FAILED!" << std::endl;
            return 1;
        }
    }

private:
    std::vector<TestCase> tests_;
};

// Helper class for automatic test registration
class TestRegistrar {
public:
    TestRegistrar(const std::string& name, std::function<void(TestResult&)> func) {
        TestSuite::instance().addTest(name, func);
    }
};

} // namespace TestFramework

// Macros for easier test definition
#define TEST(test_name) \
    void test_##test_name(TestFramework::TestResult& __result_ref); \
    static TestFramework::TestRegistrar registrar_##test_name(#test_name, test_##test_name); \
    void test_##test_name(TestFramework::TestResult& __result_ref)

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            std::ostringstream oss; \
            oss << "ASSERT_TRUE failed: " << #condition << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            std::ostringstream oss; \
            oss << "ASSERT_FALSE failed: " << #condition << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_EQ(actual, expected) \
    do { \
        if (!((expected) == (actual))) { \
            std::ostringstream oss; \
            oss << "ASSERT_EQ failed: expected " << (expected) << ", got " << (actual) \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_NE(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            std::ostringstream oss; \
            oss << "ASSERT_NE failed: both values are " << (expected) \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_NEAR(actual, expected, tolerance) \
    do { \
        auto diff = std::abs((expected) - (actual)); \
        if (diff > (tolerance)) { \
            std::ostringstream oss; \
            oss << "ASSERT_NEAR failed: expected " << (expected) << ", got " << (actual) \
                << ", diff " << diff << " > tolerance " << (tolerance) \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define RUN_ALL_TESTS() TestFramework::TestSuite::instance().runAll()
