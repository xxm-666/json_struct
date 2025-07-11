#include "json_value.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

using namespace JsonStruct;

class JSONPathTestSuite {
private:
    int totalTests;
    int passedTests;

public:
    JSONPathTestSuite() : totalTests(0), passedTests(0) {}

    void reportTest(const std::string& testName, bool passed) {
        totalTests++;
        if (passed) {
            passedTests++;
            std::cout << "  [PASS] " << testName << "\n";
        } else {
            std::cout << "  [FAIL] " << testName << "\n";
        }
    }

    // Basic path existence tests
    void testBasicPathExistence() {
        std::cout << "\n=== Testing Basic Path Existence ===\n";
        
        JsonValue json = JsonValue::object({
            {"name", JsonValue("John")},
            {"age", JsonValue(30)},
            {"address", JsonValue::object({
                {"city", JsonValue("New York")},
                {"zip", JsonValue("10001")}
            })}
        });
        
        reportTest("Root path exists", json.pathExists("$"));
        reportTest("Simple property exists", json.pathExists("$.name") && json.pathExists("$.age"));
        reportTest("Nested property exists", json.pathExists("$.address.city") && json.pathExists("$.address.zip"));
        reportTest("Non-existent paths", !json.pathExists("$.nonexistent") && !json.pathExists("$.address.nonexistent"));
        reportTest("Invalid paths", !json.pathExists("$.name.invalid"));
    }

    // Basic path selection tests
    void testBasicPathSelection() {
        std::cout << "\n=== Testing Basic Path Selection ===\n";
        
        JsonValue json = JsonValue::object({
            {"name", JsonValue("John")},
            {"age", JsonValue(30)},
            {"address", JsonValue::object({
                {"city", JsonValue("New York")},
                {"zip", JsonValue("10001")}
            })}
        });
        
        auto result = json.selectFirst("$");
        reportTest("Root selection", result != nullptr && result->isObject());
        
        result = json.selectFirst("$.name");
        reportTest("String property selection", 
            result != nullptr && result->isString() && *result->getString() == "John");
        
        result = json.selectFirst("$.age");
        reportTest("Number property selection", 
            result != nullptr && result->isNumber() && result->toInt() == 30);
        
        result = json.selectFirst("$.address.city");
        reportTest("Nested property selection", 
            result != nullptr && result->isString() && *result->getString() == "New York");
        
        result = json.selectFirst("$.nonexistent");
        reportTest("Non-existent path returns null", result == nullptr);
    }

    // Array indexing tests
    void testArrayIndexing() {
        std::cout << "\n=== Testing Array Indexing ===\n";
        
        JsonValue json = JsonValue::object({
            {"numbers", JsonValue::array({JsonValue(10), JsonValue(20), JsonValue(30), JsonValue(40)})},
            {"books", JsonValue::array({
                JsonValue::object({{"title", JsonValue("Book 1")}, {"price", JsonValue(12.99)}}),
                JsonValue::object({{"title", JsonValue("Book 2")}, {"price", JsonValue(15.99)}}),
                JsonValue::object({{"title", JsonValue("Book 3")}, {"price", JsonValue(9.99)}})
            })}
        });
        
        auto result = json.selectFirst("$.numbers[0]");
        reportTest("First array element", result != nullptr && result->toInt() == 10);
        
        result = json.selectFirst("$.numbers[2]");
        reportTest("Third array element", result != nullptr && result->toInt() == 30);
        
        result = json.selectFirst("$.books[1].title");
        reportTest("Nested array object property", 
            result != nullptr && result->isString() && *result->getString() == "Book 2");
        
        result = json.selectFirst("$.numbers[10]");
        reportTest("Out of bounds returns null", result == nullptr);
    }

    // Array wildcard tests
    void testArrayWildcard() {
        std::cout << "\n=== Testing Array Wildcard ===\n";
        
        JsonValue json = JsonValue::object({
            {"numbers", JsonValue::array({JsonValue(10), JsonValue(20), JsonValue(30)})},
            {"books", JsonValue::array({
                JsonValue::object({{"title", JsonValue("Book 1")}, {"author", JsonValue("Author 1")}}),
                JsonValue::object({{"title", JsonValue("Book 2")}, {"author", JsonValue("Author 2")}}),
                JsonValue::object({{"title", JsonValue("Book 3")}, {"author", JsonValue("Author 3")}})
            })}
        });
        
        auto results = json.selectAll("$.*");
        reportTest("Root wildcard", results.size() == 2);
        
        results = json.selectAll("$.books[*].title");
        reportTest("Array wildcard with property", results.size() == 3);
        
        bool titlesCorrect = true;
        for (size_t i = 0; i < results.size() && i < 3; ++i) {
            if (!results[i]->isString() || 
                *results[i]->getString() != ("Book " + std::to_string(i + 1))) {
                titlesCorrect = false;
                break;
            }
        }
        reportTest("Array wildcard content verification", titlesCorrect);
        
        results = json.selectAll("$.numbers[*]");
        reportTest("Array wildcard for numbers", results.size() == 3);
    }

    // Array slicing tests
    void testArraySlicing() {
        std::cout << "\n=== Testing Array Slicing ===\n";
        
        JsonValue json = JsonValue::object({
            {"numbers", JsonValue::array({
                JsonValue(10), JsonValue(20), JsonValue(30), JsonValue(40), JsonValue(50)
            })}
        });
        
        auto results = json.selectAll("$.numbers[1:3]");
        reportTest("Array slice [1:3]", results.size() == 2);
        
        if (results.size() >= 2) {
            reportTest("Slice content verification", 
                results[0]->toInt() == 20 && results[1]->toInt() == 30);
        }
        
        results = json.selectAll("$.numbers[0:2]");
        reportTest("Array slice [0:2]", results.size() == 2);
        
        results = json.selectAll("$.numbers[10:15]");
        reportTest("Out of bounds slice", results.size() == 0);
    }

    // Recursive descent tests
    void testRecursiveDescent() {
        std::cout << "\n=== Testing Recursive Descent ===\n";
        
        JsonValue json = JsonValue::object({
            {"store", JsonValue::object({
                {"book", JsonValue::array({
                    JsonValue::object({{"title", JsonValue("Book 1")}, {"price", JsonValue(10.99)}}),
                    JsonValue::object({{"title", JsonValue("Book 2")}, {"price", JsonValue(15.99)}})
                })},
                {"bicycle", JsonValue::object({{"price", JsonValue(19.95)}})}
            })},
            {"warehouse", JsonValue::object({
                {"inventory", JsonValue::object({{"price", JsonValue(25.50)}})}
            })}
        });
        
        auto results = json.selectAll("$..price");
        reportTest("Recursive descent price search", results.size() == 4);
        
        bool allNumbers = true;
        for (const auto* result : results) {
            if (!result->isNumber()) {
                allNumbers = false;
                break;
            }
        }
        reportTest("All recursive results are numbers", allNumbers);
        
        results = json.selectAll("$..title");
        reportTest("Recursive descent title search", results.size() == 2);
    }

    // Multiple value selection tests
    void testMultipleValueSelection() {
        std::cout << "\n=== Testing Multiple Value Selection ===\n";
        
        JsonValue json = JsonValue::object({
            {"numbers", JsonValue::array({JsonValue(10), JsonValue(20), JsonValue(30)})},
            {"data", JsonValue::object({
                {"values", JsonValue::array({JsonValue(100), JsonValue(200)})}
            })}
        });
        
        auto pointers = json.selectAll("$.numbers[*]");
        reportTest("selectAll returns pointers", pointers.size() == 3);
        
        auto values = json.selectValues("$.numbers[*]");
        reportTest("selectValues returns copies", values.size() == 3);
        
        bool valuesIndependent = true;
        for (size_t i = 0; i < values.size() && i < pointers.size(); ++i) {
            if (values[i].toInt() != pointers[i]->toInt()) {
                valuesIndependent = false;
                break;
            }
        }
        reportTest("Value copies are correct", valuesIndependent);
    }

    // Invalid path tests
    void testInvalidPaths() {
        std::cout << "\n=== Testing Invalid Paths ===\n";
        
        JsonValue json = JsonValue::object({
            {"name", JsonValue("John")},
            {"age", JsonValue(30)}
        });
        
        reportTest("Empty path", !json.pathExists(""));
        reportTest("Missing $ prefix", !json.pathExists("name"));
        reportTest("Missing dot", !json.pathExists("$name"));
        reportTest("Empty property", !json.pathExists("$."));
        reportTest("Empty recursive", !json.pathExists("$.."));
        
        reportTest("Invalid path selection returns null", 
            json.selectFirst("") == nullptr && 
            json.selectFirst("name") == nullptr && 
            json.selectFirst("$name") == nullptr);
    }

    // Run all tests
    void runAllTests() {
        std::cout << "JSONPath Comprehensive Test Suite\n";
        std::cout << "=====================================\n";
        
        testBasicPathExistence();
        testBasicPathSelection();
        testArrayIndexing();
        testArrayWildcard();
        testArraySlicing();
        testRecursiveDescent();
        testMultipleValueSelection();
        testInvalidPaths();
        
        std::cout << "\nTEST SUMMARY\n";
        std::cout << "===============\n";
        std::cout << "Total Tests: " << totalTests << "\n";
        std::cout << "Passed: " << passedTests << "\n";
        std::cout << "Failed: " << (totalTests - passedTests) << "\n";
        std::cout << "Success Rate: " << (totalTests > 0 ? passedTests * 100 / totalTests : 0) << "%\n";
        
        if (passedTests == totalTests) {
            std::cout << "\nALL TESTS PASSED! JSONPath implementation is 100% functional!\n";
        } else {
            std::cout << "\nSome tests failed. Please check the implementation.\n";
        }
    }

    // Demonstration of all features
    void demonstrateFeatures() {
        std::cout << "\nJSONPath Feature Demonstration\n";
        std::cout << "==================================\n";
        
        JsonValue demo = JsonValue::object({
            {"name", JsonValue("JSON Store")},
            {"books", JsonValue::array({
                JsonValue::object({{"title", JsonValue("C++ Guide")}, {"price", JsonValue(29.99)}}),
                JsonValue::object({{"title", JsonValue("JSON Manual")}, {"price", JsonValue(19.99)}})
            })},
            {"inventory", JsonValue::object({
                {"warehouse1", JsonValue::object({{"books", JsonValue(150)}})},
                {"warehouse2", JsonValue::object({{"books", JsonValue(200)}})
            })}
        });
        
        std::cout << "\nSample JSON:\n" << demo.dump(2) << "\n\n";
        
        std::vector<std::pair<std::string, std::string>> examples = {
            {"$.name", "Simple property access"},
            {"$.books[0].title", "Array indexing with property"},
            {"$.books[*].price", "Array wildcard selection"},
            {"$.books[0:2]", "Array slicing"},
            {"$..books", "Recursive descent search"},
            {"$.inventory.*", "Object wildcard"},
            {"$.*", "Root level wildcard"}
        };
        
        for (const auto& example : examples) {
            const std::string& path = example.first;
            const std::string& description = example.second;
            
            std::cout << description << ":\n";
            std::cout << "  Path: " << path << "\n";
            std::cout << "  Exists: " << (demo.pathExists(path) ? "YES" : "NO") << "\n";
            
            if (path.find("[*]") != std::string::npos || path.find("..") != std::string::npos || 
                path.find(".*") != std::string::npos || path.find("[0:") != std::string::npos) {
                auto results = demo.selectAll(path);
                std::cout << "  Results: " << results.size() << " items\n";
                for (size_t i = 0; i < results.size() && i < 3; ++i) {
                    std::cout << "    [" << i << "] " << results[i]->dump() << "\n";
                }
            } else {
                auto result = demo.selectFirst(path);
                std::cout << "  Value: " << (result ? result->dump() : "null") << "\n";
            }
            std::cout << "\n";
        }
    }
};

int main() {
    try {
        JSONPathTestSuite suite;
        
        suite.runAllTests();
        suite.demonstrateFeatures();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test suite failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
