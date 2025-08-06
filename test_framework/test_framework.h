#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <chrono>
#include <regex>
#include <algorithm>
#include <memory>
#include <exception>
#include <map>
#include <set>
#include <iomanip>

namespace TestFramework {

// Forward declarations
class TestContext;

// Test configuration and filtering
struct TestConfig {
    std::vector<std::string> includePatterns;
    std::vector<std::string> excludePatterns;
    std::vector<std::string> includeTags;
    std::vector<std::string> excludeTags;
    bool verbose = false;
    bool timing = true;
    bool stopOnFirstFailure = false;
    
    bool shouldRunTest(const std::string& testName, const std::set<std::string>& tags = {}) const {
        // Check exclude patterns first
        for (const auto& pattern : excludePatterns) {
            if (testName.find(pattern) != std::string::npos) {
                return false;
            }
        }
        
        // Check exclude tags
        for (const auto& tag : excludeTags) {
            if (tags.count(tag) > 0) {
                return false;
            }
        }
        
        // Check include patterns
        if (!includePatterns.empty()) {
            bool found = false;
            for (const auto& pattern : includePatterns) {
                if (testName.find(pattern) != std::string::npos) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        
        // Check include tags
        if (!includeTags.empty()) {
            bool found = false;
            for (const auto& tag : includeTags) {
                if (tags.count(tag) > 0) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        
        return true;
    }
};

// Enhanced test result with more detailed information
class TestResult {
public:
    TestResult() : passed_(0), failed_(0), skipped_(0), duration_(0.0) {}
    
    void addPass() { passed_++; }
    void addPass(int count) { passed_ += count; }
    void addFail(const std::string& message) {
        failed_++;
        failures_.push_back(message);
    }
    void addSkip(const std::string& reason = "") {
        skipped_++;
        if (!reason.empty()) {
            skips_.push_back(reason);
        }
    }
    void addSkip(int count) { skipped_ += count; }
    
    void setDuration(double duration) { duration_ = duration; }
    
    int getPassed() const { return passed_; }
    int getFailed() const { return failed_; }
    int getSkipped() const { return skipped_; }
    int getTotal() const { return passed_ + failed_ + skipped_; }
    double getDuration() const { return duration_; }
    
    const std::vector<std::string>& getFailures() const { return failures_; }
    const std::vector<std::string>& getSkips() const { return skips_; }
    
    bool isSuccess() const { return failed_ == 0; }
    bool hasTests() const { return getTotal() > 0; }

private:
    int passed_;
    int failed_;
    int skipped_;
    double duration_;
    std::vector<std::string> failures_;
    std::vector<std::string> skips_;
};

// Test context for setup/teardown and shared state
class TestContext {
public:
    virtual ~TestContext() = default;
    virtual void setUp() {}
    virtual void tearDown() {}
    
    // Allow tests to store and retrieve context data
    template<typename T>
    void set(const std::string& key, const T& value) {
        data_[key] = std::to_string(value);
    }
    
    void set(const std::string& key, const std::string& value) {
        data_[key] = value;
    }
    
    template<typename T>
    T get(const std::string& key, const T& defaultValue = T{}) const {
        auto it = data_.find(key);
        if (it != data_.end()) {
            std::istringstream iss(it->second);
            T value;
            iss >> value;
            return value;
        }
        return defaultValue;
    }
    
    std::string getString(const std::string& key, const std::string& defaultValue = "") const {
        auto it = data_.find(key);
        return (it != data_.end()) ? it->second : defaultValue;
    }
    
private:
    std::map<std::string, std::string> data_;
};

class TestCase {
public:
    TestCase(const std::string& name, std::function<void(TestResult&)> func, 
             const std::set<std::string>& tags = {}, bool skip = false)
        : name_(name), func_(func), tags_(tags), skip_(skip) {}

    void run(TestResult& result, TestContext* context = nullptr, bool verbose = false) {
        if (skip_) {
            result.addSkip("Test marked as skipped");
            if (verbose) {
                std::cout << "[SKIP] " << name_ << std::endl;
            }
            return;
        }

        auto start = std::chrono::high_resolution_clock::now();

        try {
            if (context) {
                context->setUp();
            }

            if (verbose) {
                std::cout << "Running " << name_ << "..." << std::endl;
            }

            func_(result);

            if (context) {
                context->tearDown();
            }

        } catch (const std::exception& e) {
            result.addFail(std::string("Unhandled exception: ") + e.what());
        } catch (...) {
            result.addFail("Unhandled unknown exception");
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double>(end - start).count();
        result.setDuration(duration);
    }

    const std::string& getName() const { return name_; }
    const std::set<std::string>& getTags() const { return tags_; }
    bool isSkipped() const { return skip_; }
    void setSkipped(bool skip) { skip_ = skip; }

private:
    std::string name_;
    std::function<void(TestResult&)> func_;
    std::set<std::string> tags_;
    bool skip_;
};

class TestSuite {
public:
    static TestSuite& instance() {
        static TestSuite suite;
        return suite;
    }
    
    void addTest(const std::string& name, std::function<void(TestResult&)> func, 
                 const std::set<std::string>& tags = {}, bool skip = false) {
        tests_.emplace_back(name, func, tags, skip);
    }
    
    void setConfig(const TestConfig& config) {
        config_ = config;
    }
    
    const TestConfig& getConfig() const {
        return config_;
    }
    
    void setContext(std::unique_ptr<TestContext> context) {
        context_ = std::move(context);
    }
    
    int runAll() {
        TestResult totalResult;
        std::cout << "=== Running Test Suite ===" << std::endl;
        
        if (config_.verbose) {
            std::cout << "Configuration:" << std::endl;
            std::cout << "  Verbose: " << (config_.verbose ? "true" : "false") << std::endl;
            std::cout << "  Timing: " << (config_.timing ? "true" : "false") << std::endl;
            std::cout << "  Stop on first failure: " << (config_.stopOnFirstFailure ? "true" : "false") << std::endl;
        }
        
        auto suiteStart = std::chrono::high_resolution_clock::now();
        
        int testsRun = 0;
        for (auto& test : tests_) {
            // Check if test should run based on configuration
            if (!config_.shouldRunTest(test.getName(), test.getTags())) {
                if (config_.verbose) {
                    std::cout << "[FILTERED] " << test.getName() << std::endl;
                }
                continue;
            }

            TestResult testResult;
            test.run(testResult, context_.get(), config_.verbose);
            testsRun++;

            // Display test result
            if (testResult.isSuccess()) {
                std::cout << "[PASS] " << test.getName();
                if (config_.timing && testResult.getDuration() > 0) {
                    std::cout << " (" << std::fixed << std::setprecision(3) 
                             << testResult.getDuration() << "s)";
                }
                std::cout << std::endl;
            } else if (testResult.getSkipped() > 0) {
                std::cout << "[SKIP] " << test.getName();
                if (!testResult.getSkips().empty()) {
                    std::cout << " - " << testResult.getSkips()[0];
                }
                std::cout << std::endl;
            } else {
                std::cout << "[FAIL] " << test.getName() << " (" 
                         << testResult.getFailed() << " failures)";
                if (config_.timing && testResult.getDuration() > 0) {
                    std::cout << " (" << std::fixed << std::setprecision(3) 
                             << testResult.getDuration() << "s)";
                }
                std::cout << std::endl;

                if (config_.verbose) {
                    for (const auto& failure : testResult.getFailures()) {
                        std::cout << "  - " << failure << std::endl;
                    }
                }

                if (config_.stopOnFirstFailure) {
                    std::cout << "Stopping on first failure as requested." << std::endl;
                    break;
                }
            }

            // Aggregate results
            totalResult.addPass(testResult.getPassed());
            totalResult.addSkip(testResult.getSkipped());
            for (const auto& failure : testResult.getFailures()) {
                totalResult.addFail(failure);
            }
        }
        
        auto suiteEnd = std::chrono::high_resolution_clock::now();
        auto suiteDuration = std::chrono::duration<double>(suiteEnd - suiteStart).count();
        
        // Print summary
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Tests run: " << testsRun << std::endl;
        std::cout << "Total: " << totalResult.getTotal() << std::endl;
        std::cout << "Passed: " << totalResult.getPassed() << std::endl;
        std::cout << "Failed: " << totalResult.getFailed() << std::endl;
        std::cout << "Skipped: " << totalResult.getSkipped() << std::endl;
        
        if (config_.timing) {
            std::cout << "Total time: " << std::fixed << std::setprecision(3) 
                     << suiteDuration << "s" << std::endl;
        }
        
        if (totalResult.isSuccess()) {
            std::cout << "ALL TESTS PASSED!" << std::endl;
            return 0;
        } else {
            std::cout << "SOME TESTS FAILED!" << std::endl;
            
            if (!config_.verbose) {
                std::cout << "\n=== Failure Details ===" << std::endl;
                for (const auto& failure : totalResult.getFailures()) {
                    std::cout << "- " << failure << std::endl;
                }
            }
            return 1;
        }
    }
    
    size_t getTestCount() const { return tests_.size(); }
    
    void listTests() const {
        std::cout << "Available tests:" << std::endl;
        for (const auto& test : tests_) {
            std::cout << "  " << test.getName();
            if (!test.getTags().empty()) {
                std::cout << " [";
                bool first = true;
                for (const auto& tag : test.getTags()) {
                    if (!first) std::cout << ", ";
                    std::cout << tag;
                    first = false;
                }
                std::cout << "]";
            }
            if (test.isSkipped()) {
                std::cout << " (skipped)";
            }
            std::cout << std::endl;
        }
    }

private:
    std::vector<TestCase> tests_;
    TestConfig config_;
    std::unique_ptr<TestContext> context_;
};

// Helper class for automatic test registration
class TestRegistrar {
public:
    TestRegistrar(const std::string& name, std::function<void(TestResult&)> func, 
                  const std::set<std::string>& tags = {}, bool skip = false) {
        TestSuite::instance().addTest(name, func, tags, skip);
    }
};

// Exception for test skipping
class TestSkipException : public std::exception {
public:
    explicit TestSkipException(const std::string& reason) : reason_(reason) {}
    const char* what() const noexcept override { return reason_.c_str(); }
private:
    std::string reason_;
};

} // namespace TestFramework

// Enhanced macros for easier test definition

// Basic test definition
#define TEST(test_name) \
    void test_##test_name(TestFramework::TestResult& __result_ref); \
    static TestFramework::TestRegistrar registrar_##test_name(#test_name, test_##test_name); \
    void test_##test_name(TestFramework::TestResult& __result_ref)

// Test with tags
#define TEST_WITH_TAGS(test_name, ...) \
    void test_##test_name(TestFramework::TestResult& __result_ref); \
    static TestFramework::TestRegistrar registrar_##test_name(#test_name, test_##test_name, {__VA_ARGS__}); \
    void test_##test_name(TestFramework::TestResult& __result_ref)

// Skipped test
#define TEST_SKIP(test_name, reason) \
    void test_##test_name(TestFramework::TestResult& __result_ref) { \
        __result_ref.addSkip(reason); \
    } \
    static TestFramework::TestRegistrar registrar_##test_name(#test_name, test_##test_name, {}, true);

// Skip current test
#define SKIP_TEST(reason) \
    do { \
        __result_ref.addSkip(reason); \
        return; \
    } while(0)

// Basic assertions (existing)
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

// Enhanced string assertions
#define ASSERT_STREQ(actual, expected) \
    do { \
        std::string actual_str = (actual); \
        std::string expected_str = (expected); \
        if (actual_str != expected_str) { \
            std::ostringstream oss; \
            oss << "ASSERT_STREQ failed: expected \"" << expected_str << "\", got \"" << actual_str \
                << "\" at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_STRNE(actual, expected) \
    do { \
        std::string actual_str = (actual); \
        std::string expected_str = (expected); \
        if (actual_str == expected_str) { \
            std::ostringstream oss; \
            oss << "ASSERT_STRNE failed: both strings are \"" << expected_str \
                << "\" at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_CONTAINS(haystack, needle) \
    do { \
        std::string haystack_str = (haystack); \
        std::string needle_str = (needle); \
        if (haystack_str.find(needle_str) == std::string::npos) { \
            std::ostringstream oss; \
            oss << "ASSERT_CONTAINS failed: \"" << haystack_str << "\" does not contain \"" << needle_str \
                << "\" at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_NOT_CONTAINS(haystack, needle) \
    do { \
        std::string haystack_str = (haystack); \
        std::string needle_str = (needle); \
        if (haystack_str.find(needle_str) != std::string::npos) { \
            std::ostringstream oss; \
            oss << "ASSERT_NOT_CONTAINS failed: \"" << haystack_str << "\" contains \"" << needle_str \
                << "\" at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_STARTS_WITH(localstr, prefix) \
    do { \
        std::string str_val = (localstr); \
        std::string prefix_val = (prefix); \
        if (str_val.substr(0, prefix_val.length()) != prefix_val) { \
            std::ostringstream oss; \
            oss << "ASSERT_STARTS_WITH failed: \"" << str_val << "\" does not start with \"" << prefix_val \
                << "\" at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_ENDS_WITH(localstr, suffix) \
    do { \
        std::string str_val = (localstr); \
        std::string suffix_val = (suffix); \
        if (str_val.length() < suffix_val.length() || \
            str_val.substr(str_val.length() - suffix_val.length()) != suffix_val) { \
            std::ostringstream oss; \
            oss << "ASSERT_ENDS_WITH failed: \"" << str_val << "\" does not end with \"" << suffix_val \
                << "\" at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_MATCHES_REGEX(localstr, pattern) \
    do { \
        std::string str_val = (localstr); \
        std::string pattern_val = (pattern); \
        try { \
            std::regex regex_pattern(pattern_val); \
            if (!std::regex_match(str_val, regex_pattern)) { \
                std::ostringstream oss; \
                oss << "ASSERT_MATCHES_REGEX failed: \"" << str_val << "\" does not match pattern \"" << pattern_val \
                    << "\" at " << __FILE__ << ":" << __LINE__; \
                __result_ref.addFail(oss.str()); \
                return; \
            } else { \
                __result_ref.addPass(); \
            } \
        } catch (const std::exception& e) { \
            std::ostringstream oss; \
            oss << "ASSERT_MATCHES_REGEX failed: invalid regex pattern \"" << pattern_val << "\": " << e.what() \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } \
    } while(0)

// Container assertions
#define ASSERT_EMPTY(container) \
    do { \
        if (!(container).empty()) { \
            std::ostringstream oss; \
            oss << "ASSERT_EMPTY failed: container is not empty (size=" << (container).size() << ")" \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_NOT_EMPTY(container) \
    do { \
        if ((container).empty()) { \
            std::ostringstream oss; \
            oss << "ASSERT_NOT_EMPTY failed: container is empty" \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_SIZE_EQ(container, expected_size) \
    do { \
        size_t actual_size = (container).size(); \
        size_t expected = (expected_size); \
        if (actual_size != expected) { \
            std::ostringstream oss; \
            oss << "ASSERT_SIZE_EQ failed: expected size " << expected << ", got " << actual_size \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

// Exception assertions
#define ASSERT_THROWS(statement, exception_type) \
    do { \
        bool caught_expected_exception = false; \
        try { \
            statement; \
        } catch (const exception_type&) { \
            caught_expected_exception = true; \
        } catch (...) { \
            std::ostringstream oss; \
            oss << "ASSERT_THROWS failed: caught unexpected exception type" \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } \
        if (!caught_expected_exception) { \
            std::ostringstream oss; \
            oss << "ASSERT_THROWS failed: expected " << #exception_type << " was not thrown" \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_NO_THROW(statement) \
    do { \
        try { \
            statement; \
            __result_ref.addPass(); \
        } catch (...) { \
            std::ostringstream oss; \
            oss << "ASSERT_NO_THROW failed: unexpected exception was thrown" \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } \
    } while(0)

#define ASSERT_ANY_THROW(statement) \
    do { \
        bool caught_exception = false; \
        try { \
            statement; \
        } catch (...) { \
            caught_exception = true; \
        } \
        if (!caught_exception) { \
            std::ostringstream oss; \
            oss << "ASSERT_ANY_THROW failed: expected exception was not thrown" \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

// Pointer assertions
#define ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != nullptr) { \
            std::ostringstream oss; \
            oss << "ASSERT_NULL failed: pointer is not null" \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == nullptr) { \
            std::ostringstream oss; \
            oss << "ASSERT_NOT_NULL failed: pointer is null" \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

// Additional numeric assertions  
#define ASSERT_LT(left, right) \
    do { \
        if (!((left) < (right))) { \
            std::ostringstream oss; \
            oss << "ASSERT_LT failed: " << (left) << " is not less than " << (right) \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_LE(left, right) \
    do { \
        if (!((left) <= (right))) { \
            std::ostringstream oss; \
            oss << "ASSERT_LE failed: " << (left) << " is not less than or equal to " << (right) \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_GT(left, right) \
    do { \
        if (!((left) > (right))) { \
            std::ostringstream oss; \
            oss << "ASSERT_GT failed: " << (left) << " is not greater than " << (right) \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

#define ASSERT_GE(left, right) \
    do { \
        if (!((left) >= (right))) { \
            std::ostringstream oss; \
            oss << "ASSERT_GE failed: " << (left) << " is not greater than or equal to " << (right) \
                << " at " << __FILE__ << ":" << __LINE__; \
            __result_ref.addFail(oss.str()); \
            return; \
        } else { \
            __result_ref.addPass(); \
        } \
    } while(0)

// Main runner macros
#define RUN_ALL_TESTS() TestFramework::TestSuite::instance().runAll()

#define SET_TEST_CONFIG(config) TestFramework::TestSuite::instance().setConfig(config)

#define LIST_TESTS() TestFramework::TestSuite::instance().listTests()
