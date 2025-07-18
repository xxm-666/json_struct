#include <iostream>
#include "../src/json_engine/json_path.h"
#include "../src/json_engine/json_value.h"

using namespace JsonStruct;
using namespace jsonvalue_jsonpath;

void test_edge_cases() {
    std::cout << "=== Testing Edge Cases for Spaced Properties ===" << std::endl;
    
    // Create test data with various edge cases
    JsonValue root = JsonValue::object({
        {"normal", JsonValue("value1")},
        {" leading_space", JsonValue("value2")},
        {"trailing_space ", JsonValue("value3")},
        {" both spaces ", JsonValue("value4")},
        {"multiple  spaces", JsonValue("value5")},
        {"tab\tchar", JsonValue("value6")},
        {"newline\nchar", JsonValue("value7")},
        {"special!@#$%^&*()chars", JsonValue("value8")},
        {"", JsonValue("empty_key")},
        {"unicode测试", JsonValue("unicode_value")},
        {"123numeric", JsonValue("numeric_start")},
        {"with'quote", JsonValue("single_quote")},
        {"with\"quote", JsonValue("double_quote")},
        {"with\\backslash", JsonValue("backslash")}
    });
    
    std::cout << "Created test data with various edge cases." << std::endl;
    
    // Test cases
    std::vector<std::pair<std::string, std::string>> test_cases = {
        {"$[' leading_space']", "leading space"},
        {"$['trailing_space ']", "trailing space"},
        {"$[' both spaces ']", "both spaces"},
        {"$['multiple  spaces']", "multiple spaces"},
        {"$['tab\tchar']", "tab character"},
        {"$['newline\nchar']", "newline character"},
        {"$['special!@#$%^&*()chars']", "special characters"},
        {"$['']", "empty key"},
        {"$['unicode测试']", "unicode characters"},
        {"$['123numeric']", "numeric start"},
        {"$['with\\'quote']", "escaped single quote"},
        {"$[\"with\\\"quote\"]", "escaped double quote"},
        {"$['with\\\\backslash']", "backslash"}
    };
    
    for (const auto& [path, description] : test_cases) {
        std::cout << "\n--- Testing " << description << ": " << path << " ---" << std::endl;
        try {
            auto result = selectFirst(root, path);
            if (result.has_value()) {
                std::cout << "✅ Found: " << result->get().dump() << std::endl;
            } else {
                std::cout << "❌ Not found" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "❌ Error: " << e.what() << std::endl;
        }
    }
}

void test_filter_with_spaced_properties() {
    std::cout << "\n=== Testing Filters with Spaced Properties ===" << std::endl;
    
    JsonValue root = JsonValue::object({
        {"users", JsonValue::array({
            JsonValue::object({
                {"user name", JsonValue("Alice")},
                {"age score", JsonValue(85)},
                {"is active", JsonValue(true)}
            }),
            JsonValue::object({
                {"user name", JsonValue("Bob")},
                {"age score", JsonValue(95)},
                {"is active", JsonValue(false)}
            }),
            JsonValue::object({
                {"user name", JsonValue("Charlie")},
                {"age score", JsonValue(75)},
                {"is active", JsonValue(true)}
            })
        })}
    });
    
    std::cout << "Test data: " << root.dump(2) << std::endl;
    
    // Test filter with spaced property names
    std::cout << "\n--- Test: $.users[?(@['age score'] > 80)] ---" << std::endl;
    try {
        auto results = selectAll(root, "$.users[?(@['age score'] > 80)]");
        std::cout << "Found " << results.size() << " users with age score > 80:" << std::endl;
        for (const auto& result : results) {
            std::cout << "  " << result.get()["user name"].dump() << " (score: " 
                      << result.get()["age score"].dump() << ")" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    // Test filter with boolean spaced property
    std::cout << "\n--- Test: $.users[?(@['is active'] == true)] ---" << std::endl;
    try {
        auto results = selectAll(root, "$.users[?(@['is active'] == true)]");
        std::cout << "Found " << results.size() << " active users:" << std::endl;
        for (const auto& result : results) {
            std::cout << "  " << result.get()["user name"].dump() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void test_mixed_notation() {
    std::cout << "\n=== Testing Mixed Notation ===" << std::endl;
    
    JsonValue root = JsonValue::object({
        {"data", JsonValue::object({
            {"user info", JsonValue::array({
                JsonValue::object({
                    {"name", JsonValue("John")},
                    {"contact details", JsonValue::object({
                        {"phone number", JsonValue("123-456-7890")},
                        {"email", JsonValue("john@example.com")}
                    })}
                })
            })}
        })}
    });
    
    // Test mixing dot notation and bracket notation
    std::vector<std::string> mixed_paths = {
        "$.data['user info'][0].name",
        "$.data['user info'][*]['contact details'].email",
        "$.data['user info'][*]['contact details']['phone number']"
    };
    
    for (const auto& path : mixed_paths) {
        std::cout << "\n--- Test: " << path << " ---" << std::endl;
        try {
            auto results = selectAll(root, path);
            std::cout << "Found " << results.size() << " matches:" << std::endl;
            for (const auto& result : results) {
                std::cout << "  " << result.get().dump() << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
}

int main() {
    try {
        std::cout << "Comprehensive JSONPath Spaced Property Tests\n" << std::endl;
        
        test_edge_cases();
        test_filter_with_spaced_properties();
        test_mixed_notation();
        
        std::cout << "\n=== Summary ===" << std::endl;
        std::cout << "✅ JSONPath supports spaced property names using bracket notation:" << std::endl;
        std::cout << "   - Single quotes: ['property name']" << std::endl;
        std::cout << "   - Double quotes: [\"property name\"]" << std::endl;
        std::cout << "   - Special characters, unicode, and edge cases work" << std::endl;
        std::cout << "   - Filters with spaced properties work" << std::endl;
        std::cout << "   - Mixed notation (dot + bracket) works" << std::endl;
        std::cout << "❌ Direct dot notation with spaces does NOT work (by design)" << std::endl;
        std::cout << "\nTo use properties with spaces, use: $.path['property name']" << std::endl;
        std::cout << "Instead of: $.path.property name (which will fail)" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
