#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

struct TestResult {
    std::string testName;
    bool passed{};
    int totalTests{};
    int passedTests{};
    int failedTests{};
    double duration{};
    std::string output;
};

class TestRunner {
private:
    std::vector<std::string> testExecutables;
    std::vector<TestResult> results;

public:
    TestRunner() {
        testExecutables = {
            "rfc7396_test.exe",
            "test_complex_nested_containers.exe",
            "test_complex_structures.exe",
            "test_core_functionality.exe",
            "test_custom_types.exe",
            "test_enhanced_features.exe",
            "test_error_codes.exe",
            "test_error_handling.exe",
            "test_error_recovery.exe",
            "test_extreme_exception_scenarios.exe",
            "test_jsonpath_comprehensive.exe",
            "test_json_auto.exe",
            "test_json_parsing.exe",
            "test_json_path_mutable.exe",
            "test_json_pipeline.exe",
            "test_json_pointer.exe",
            "test_json_pointer_basic.exe",
            "test_json_query_generator.exe",
            "test_large_deep_nested_objects.exe",
            "test_lazy_performance_comprehensive.exe",
            "test_lazy_query_comprehensive.exe",
            "test_lazy_suite_runner.exe",
            // "test_multithreaded_concurrency.exe",
            // "test_performance_limits.exe",
            "test_performance_struct.exe",
            "test_serialization_basic.exe",
            "test_serialization_options.exe",
            "test_spaced_properties.exe",
            "test_special_numbers.exe",
            "test_type_conversion_boundary.exe"
        };
    }

    std::string runCommand(const std::string& command) {
        std::string result;
        
#ifdef _WIN32
        HANDLE hPipeRead, hPipeWrite;
        SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES)};
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        if (CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
            STARTUPINFOA si = {sizeof(STARTUPINFOA)};
            si.dwFlags = STARTF_USESTDHANDLES;
            si.hStdOutput = hPipeWrite;
            si.hStdError = hPipeWrite;

            PROCESS_INFORMATION pi = {0};
            
            if (CreateProcessA(NULL, const_cast<char*>(command.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
                CloseHandle(hPipeWrite);
                
                char buffer[4096];
                DWORD bytesRead;
                while (ReadFile(hPipeRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    result += buffer;
                }
                std::cout << "result: " << result << std::endl;
                WaitForSingleObject(pi.hProcess, INFINITE);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
            CloseHandle(hPipeRead);
        }
#else
        FILE* pipe = popen(command.c_str(), "r");
        if (pipe) {
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                result += buffer;
            }
            pclose(pipe);
        }
#endif
        return result;
    }

    TestResult parseTestOutput(const std::string& testName, const std::string& output) {
        TestResult result;
        result.testName = testName;
        result.output = output;
        result.passed = false;
        result.totalTests = 0;
        result.passedTests = 0;
        result.failedTests = 0;

        std::istringstream iss(output);
        std::string line;
        
        while (std::getline(iss, line)) {
            //Search "Total: X, Passed: Y, Failed: Z"
            if (line.find("Total:") != std::string::npos && 
                line.find("Passed:") != std::string::npos) {
                
                size_t totalPos = line.find("Total:");
                size_t passedPos = line.find("Passed:");
                size_t failedPos = line.find("Failed:");
                
                if (totalPos != std::string::npos && passedPos != std::string::npos) {
                    try {
                        std::string totalStr = line.substr(totalPos + 6);
                        size_t commaPos = totalStr.find(',');
                        if (commaPos != std::string::npos) {
                            totalStr = totalStr.substr(0, commaPos);
                        }
                        /// remove leading and trailing whitespace
                        totalStr.erase(0, totalStr.find_first_not_of(" \t"));
                        totalStr.erase(totalStr.find_last_not_of(" \t") + 1);
                        result.totalTests = std::stoi(totalStr);
                        
                        std::string passedStr = line.substr(passedPos + 7);
                        commaPos = passedStr.find(',');
                        if (commaPos != std::string::npos) {
                            passedStr = passedStr.substr(0, commaPos);
                        }
                        passedStr.erase(0, passedStr.find_first_not_of(" \t"));
                        passedStr.erase(passedStr.find_last_not_of(" \t") + 1);
                        result.passedTests = std::stoi(passedStr);
                        
                        if (failedPos != std::string::npos) {
                            std::string failedStr = line.substr(failedPos + 7);
                            commaPos = failedStr.find(',');
                            if (commaPos != std::string::npos) {
                                failedStr = failedStr.substr(0, commaPos);
                            }
                            failedStr.erase(0, failedStr.find_first_not_of(" \t"));
                            failedStr.erase(failedStr.find_last_not_of(" \t") + 1);
                            result.failedTests = std::stoi(failedStr);
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Failed to parse test counts for " << testName << ": " << e.what() << std::endl;
                    }
                }
            }
            
            if (line.find("ALL TESTS PASSED!") != std::string::npos) {
                result.passed = true;
            } else if (line.find("SOME TESTS FAILED!") != std::string::npos) {
                result.passed = false;
            }
        }

        return result;
    }

    void runAllTests() {
        std::cout << "=== JsonStruct Test Suite Runner ===" << std::endl;
        std::cout << "Starting automated test execution..." << std::endl;
        std::cout << std::endl;

        auto startTime = std::chrono::high_resolution_clock::now();

        for (const auto& testExe : testExecutables) {
            std::cout << "Running " << testExe << "... ";
            std::cout.flush();

            auto testStart = std::chrono::high_resolution_clock::now();
            std::string output = runCommand(testExe);
            auto testEnd = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration<double>(testEnd - testStart).count();
            
            TestResult result = parseTestOutput(testExe, output);
            result.duration = duration;
            results.push_back(result);

            if (result.passed) {
                std::cout << "PASSED";
            } else {
                std::cout << "FAILED";
            }
            std::cout << " (" << std::fixed << std::setprecision(2) << duration << "s)" << std::endl;
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        auto totalDuration = std::chrono::duration<double>(endTime - startTime).count();

        printSummary(totalDuration);
    }

    void printSummary(double totalDuration) {
        std::cout << std::endl;
        std::cout << "=== Test Summary ===" << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        
        int totalTestsPassed = 0;
        int totalTestsFailed = 0;
        int totalTestCount = 0;
        int suitesPasssed = 0;
        int suitesFailed = 0;

        for (const auto& result : results) {
            totalTestsPassed += result.passedTests;
            totalTestsFailed += result.failedTests;
            totalTestCount += result.totalTests;
            
            if (result.passed) {
                suitesPasssed++;
            } else {
                suitesFailed++;
            }

            std::cout << std::left << std::setw(30) << result.testName 
                      << " | " << std::setw(6) << (result.passed ? "PASS" : "FAIL")
                      << " | " << std::setw(3) << result.totalTests << " tests"
                      << " | " << std::fixed << std::setprecision(2) << std::setw(6) << result.duration << "s"
                      << std::endl;
        }

        std::cout << std::string(80, '=') << std::endl;
        std::cout << "Total Test Suites: " << results.size() << std::endl;
        std::cout << "Suites Passed: " << suitesPasssed << std::endl;
        std::cout << "Suites Failed: " << suitesFailed << std::endl;
        std::cout << std::endl;
        std::cout << "Total Individual Tests: " << totalTestCount << std::endl;
        std::cout << "Individual Tests Passed: " << totalTestsPassed << std::endl;
        std::cout << "Individual Tests Failed: " << totalTestsFailed << std::endl;
        std::cout << std::endl;
        std::cout << "Total Execution Time: " << std::fixed << std::setprecision(2) << totalDuration << " seconds" << std::endl;

        if (suitesFailed == 0) {
            std::cout << std::endl;
            std::cout << "ALL TEST SUITES PASSED!" << std::endl;
        } else {
            std::cout << std::endl;
            std::cout << " " << suitesFailed << " TEST SUITE(S) FAILED!" << std::endl;
            
            std::cout << std::endl;
            std::cout << "=== Failed Test Details ===" << std::endl;
            for (const auto& result : results) {
                if (!result.passed) {
                    std::cout << std::endl;
                    std::cout << "--- " << result.testName << " ---" << std::endl;
                    std::cout << result.output << std::endl;
                }
            }
        }
    }
};

int main() {
    TestRunner runner;
    runner.runAllTests();
    return 0;
}
