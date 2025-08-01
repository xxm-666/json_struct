#include <iostream>
#include "../src/json_engine/json_path.h"
#include "../src/json_engine/json_value.h"

using namespace JsonStruct;
using namespace jsonvalue_jsonpath;

void test_spaced_property_names() {
    std::cout << "=== Testing Spaced Property Names ===" << std::endl;
    
    // Create test JSON with spaced property names
    JsonValue root = JsonValue::object({
        {"x", JsonValue::array({
            JsonValue::object({
                {"test a", JsonValue("value1")},
                {"normal_prop", JsonValue("normal1")}
            }),
            JsonValue::object({
                {"test a", JsonValue("value2")},
                {"normal_prop", JsonValue("normal2")}
            }),
            JsonValue::object({
                {"test a", JsonValue("value3")},
                {"another prop", JsonValue("another_value")}
            })
        })}
    });
    
    std::cout << "Test data: " << root.dump(2) << std::endl;
    
    // Test 1: Using bracket notation with quotes for spaced property name
    std::cout << "\n--- Test 1: $.x[*]['test a'] ---" << std::endl;
    try {
        auto results = selectAll(root, "$.x[*]['test a']");
        std::cout << "Found " << results.size() << " matches:" << std::endl;
        for (const auto& result : results) {
            std::cout << "  " << result.get().dump() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    // Test 2: Using bracket notation with double quotes
    std::cout << "\n--- Test 2: $.x[*][\"test a\"] ---" << std::endl;
    try {
        auto results = selectAll(root, "$.x[*][\"test a\"]");
        std::cout << "Found " << results.size() << " matches:" << std::endl;
        for (const auto& result : results) {
            std::cout << "  " << result.get().dump() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    // Test 3: Try direct dot notation (should fail)
    std::cout << "\n--- Test 3: $.x[*].test a (should fail) ---" << std::endl;
    try {
        auto results = selectAll(root, "$.x[*].test a");
        std::cout << "Found " << results.size() << " matches:" << std::endl;
        for (const auto& result : results) {
            std::cout << "  " << result.get().dump() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Expected error: " << e.what() << std::endl;
    }
    
    // Test 4: Another property with space
    std::cout << "\n--- Test 4: $.x[*]['another prop'] ---" << std::endl;
    try {
        auto results = selectAll(root, "$.x[*]['another prop']");
        std::cout << "Found " << results.size() << " matches:" << std::endl;
        for (const auto& result : results) {
            std::cout << "  " << result.get().dump() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    // Test 5: Mutable operations with spaced property names
    std::cout << "\n--- Test 5: Mutable operations with spaced names ---" << std::endl;
    try {
        auto mutable_results = selectAllMutable(root, "$.x[*]['test a']");
        std::cout << "Found " << mutable_results.size() << " mutable references" << std::endl;
        
        // Modify values
        for (auto& ref : mutable_results) {
            std::string old_value = ref.get().toString();
            ref.get() = JsonValue(old_value + "_modified");
        }
        
        std::cout << "After modification: " << root.dump(2) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void test_complex_spaced_paths() {
    std::cout << "\n=== Testing Complex Spaced Paths ===" << std::endl;
    
    JsonValue root = JsonValue::object({
        {"data source", JsonValue::object({
            {"user info", JsonValue::array({
                JsonValue::object({
                    {"first name", JsonValue("John")},
                    {"last name", JsonValue("Doe")},
                    {"contact details", JsonValue::object({
                        {"home address", JsonValue("123 Main St")},
                        {"work phone", JsonValue("555-1234")}
                    })}
                }),
                JsonValue::object({
                    {"first name", JsonValue("Jane")},
                    {"last name", JsonValue("Smith")},
                    {"contact details", JsonValue::object({
                        {"home address", JsonValue("456 Oak Ave")},
                        {"work phone", JsonValue("555-5678")}
                    })}
                })
            })}
        })}
    });
    
    std::cout << "Complex test data created." << std::endl;
    
    // Test deeply nested spaced property access
    std::cout << "\n--- Test: $['data source']['user info'][*]['first name'] ---" << std::endl;
    try {
        auto results = selectAll(root, "$['data source']['user info'][*]['first name']");
        std::cout << "Found " << results.size() << " first names:" << std::endl;
        for (const auto& result : results) {
            std::cout << "  " << result.get().dump() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    // Test recursive with spaced properties
    std::cout << "\n--- Test: $..['work phone'] ---" << std::endl;
    try {
        auto results = selectAll(root, "$..['work phone']");
        std::cout << "Found " << results.size() << " work phones:" << std::endl;
        for (const auto& result : results) {
            std::cout << "  " << result.get().dump() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

int main() {
    try {
        std::cout << "JSONPath Spaced Property Names Test\n" << std::endl;
        
        test_spaced_property_names();
        test_complex_spaced_paths();
        
        std::cout << "\nAll spaced property name tests completed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
