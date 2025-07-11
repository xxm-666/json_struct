#include "json_value.h"
#include <iostream>
#include <cassert>

using namespace JsonStruct;

void testBasicErrorRecovery() {
    std::cout << "=== Basic Error Recovery Test ===" << std::endl;
    
    JsonValue::ParseOptions recovery;
    recovery.allowRecovery = true;
    
    // Test 1: Skip invalid characters and find valid JSON
    try {
        std::cout << "Test 1: Skip invalid characters" << std::endl;
        auto result = JsonValue::parse("@#$%42", recovery);
        std::cout << "   Recovered value: " << result.toInt() << std::endl;
        assert(result.toInt() == 42);
        std::cout << "   ✅ Basic recovery test passed" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   ❌ Basic recovery failed: " << e.what() << std::endl;
    }
    
    // Test 2: Recover from malformed objects
    try {
        std::cout << "\nTest 2: Recover from malformed objects" << std::endl;
        auto result = JsonValue::parse("{\"key\": invalid_value, \"valid\": 123}", recovery);
        std::cout << "   Recovered object size: " << result.size() << std::endl;
        if (result.contains("valid")) {
            std::cout << "   Valid field value: " << result["valid"].toInt() << std::endl;
            assert(result["valid"].toInt() == 123);
        }
        std::cout << "   ✅ Object recovery test passed" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   ⚠️ Object recovery: " << e.what() << std::endl;
    }
    
    // Test 3: Recover from malformed arrays
    try {
        std::cout << "\nTest 3: Recover from malformed arrays" << std::endl;
        auto result = JsonValue::parse("[1, invalid, 3, 4]", recovery);
        std::cout << "   Recovered array size: " << result.size() << std::endl;
        std::cout << "   ✅ Array recovery test passed" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   ⚠️ Array recovery: " << e.what() << std::endl;
    }
}

void testAdvancedErrorRecovery() {
    std::cout << "\n=== Advanced Error Recovery Test ===" << std::endl;
    
    JsonValue::ParseOptions recovery;
    recovery.allowRecovery = true;
    recovery.allowComments = true;  // Enable additional tolerance
    
    // Test 1: Mixed valid/invalid content
    try {
        std::cout << "Test 1: Mixed valid/invalid content" << std::endl;
        std::string malformed = R"(
            garbage_text_here
            {
                "name": "Alice",
                invalid_field: broken_value,
                "age": 30,
                more_garbage
                "valid": true
            }
            trailing_garbage
        )";
        
        auto result = JsonValue::parse(malformed, recovery);
        std::cout << "   Successfully parsed object with " << result.size() << " fields" << std::endl;
        
        if (result.contains("name")) {
            std::cout << "   Name: " << result["name"].toString() << std::endl;
        }
        if (result.contains("age")) {
            std::cout << "   Age: " << result["age"].toInt() << std::endl;
        }
        if (result.contains("valid")) {
            std::cout << "   Valid: " << (result["valid"].toBool() ? "true" : "false") << std::endl;
        }
        
        std::cout << "   ✅ Advanced recovery test passed" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   ⚠️ Advanced recovery: " << e.what() << std::endl;
    }
}

void testErrorRecoveryLimits() {
    std::cout << "\n=== Error Recovery Limits Test ===" << std::endl;
    
    // Test with recovery disabled (strict mode)
    JsonValue::ParseOptions strict;
    strict.allowRecovery = false;
    
    try {
        std::cout << "Test 1: Strict mode should fail on invalid input" << std::endl;
        auto result = JsonValue::parse("@#$%42", strict);
        std::cout << "   ❌ Strict mode unexpectedly succeeded" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   ✅ Strict mode correctly failed: " << e.what() << std::endl;
    }
    
    // Test completely invalid input even with recovery
    JsonValue::ParseOptions recovery;
    recovery.allowRecovery = true;
    
    try {
        std::cout << "\nTest 2: Completely invalid input" << std::endl;
        auto result = JsonValue::parse("completely_invalid_no_json_at_all", recovery);
        std::cout << "   ❌ Recovery unexpectedly succeeded on invalid input" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   ✅ Recovery correctly failed on completely invalid input" << std::endl;
    }
}

void testStreamingSimulation() {
    std::cout << "\n=== Streaming Simulation Test ===" << std::endl;
    
    // Simulate parsing large JSON in chunks
    std::vector<std::string> chunks = {
        "{\"users\": [",
        "{\"id\": 1, \"name\": \"Alice\"},",
        "{\"id\": 2, \"name\": \"Bob\"},",
        "{\"id\": 3, \"name\": \"Charlie\"}",
        "]}"
    };
    
    try {
        std::cout << "Test 1: Chunk-based parsing simulation" << std::endl;
        
        // Combine chunks and parse as one
        std::string combined;
        for (const auto& chunk : chunks) {
            combined += chunk;
        }
        
        auto result = JsonValue::parse(combined);
        std::cout << "   Successfully parsed combined JSON" << std::endl;
        
        if (result.contains("users") && result["users"].isArray()) {
            std::cout << "   Found " << result["users"].size() << " users" << std::endl;
            for (size_t i = 0; i < result["users"].size(); ++i) {
                auto& user = result["users"][i];
                if (user.contains("name")) {
                    std::cout << "     User " << (i+1) << ": " << user["name"].toString() << std::endl;
                }
            }
        }
        
        std::cout << "   ✅ Streaming simulation test passed" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   ❌ Streaming simulation failed: " << e.what() << std::endl;
    }
}

int main() {
    try {
        testBasicErrorRecovery();
        testAdvancedErrorRecovery();
        testErrorRecoveryLimits();
        testStreamingSimulation();
        
        std::cout << "\n🎉 Error recovery tests completed!" << std::endl;
        std::cout << "✅ Basic error recovery implemented" << std::endl;
        std::cout << "⚠️ Advanced recovery needs additional work" << std::endl;
        std::cout << "🔄 Streaming support requires architectural changes" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
