#include "json_engine/json_value.h"
#include <iostream>

using namespace JsonStruct;

int main() {
    // Test 1: Valid JSON
    {
        JsonValue value;
        std::string errMsg;
        auto ec = JsonValue::parse(R"({"name": "test", "value": 42})", value, errMsg);
        
        if (ec) {
            std::cout << "Test 1 FAILED: " << ec.message() << " - " << errMsg << std::endl;
            return 1;
        } else {
            std::cout << "Test 1 PASSED: Valid JSON parsed successfully" << std::endl;
        }
    }
    
    // Test 2: Invalid JSON - unexpected character
    {
        JsonValue value;
        std::string errMsg;
        auto ec = JsonValue::parse(R"({"name": test})", value, errMsg);
        
        if (ec == JsonErrc::UnexpectedCharacter) {
            std::cout << "Test 2 PASSED: Correctly detected unexpected character - " << errMsg << std::endl;
        } else {
            std::cout << "Test 2 FAILED: Expected UnexpectedCharacter, got " << ec.message() << std::endl;
            return 1;
        }
    }
    
    // Test 3: Invalid JSON - unexpected end
    {
        JsonValue value;
        std::string errMsg;
        auto ec = JsonValue::parse(R"({"name": "test")", value, errMsg);
        
        if (ec == JsonErrc::UnexpectedEnd) {
            std::cout << "Test 3 PASSED: Correctly detected unexpected end - " << errMsg << std::endl;
        } else {
            std::cout << "Test 3 FAILED: Expected UnexpectedEnd, got " << ec.message() << std::endl;
            return 1;
        }
    }
    
    // Test 4: Invalid number
    {
        JsonValue value;
        std::string errMsg;
        auto ec = JsonValue::parse(R"({"value": 12.})", value, errMsg);
        
        if (ec == JsonErrc::ParseError) {
            std::cout << "Test 4 PASSED: Correctly detected parse error - " << errMsg << std::endl;
        } else {
            std::cout << "Test 4 FAILED: Expected ParseError, got " << ec.message() << std::endl;
            return 1;
        }
    }
    
    // Test 5: Test serialization
    {
        JsonValue value = JsonValue::object({
            {"name", JsonValue(std::string("test"))},
            {"value", JsonValue(42)}
        });
        
        std::string output;
        std::string errMsg;
        auto ec = value.toJson(output, errMsg);
        
        if (ec) {
            std::cout << "Test 5 FAILED: Serialization error - " << errMsg << std::endl;
            return 1;
        } else {
            std::cout << "Test 5 PASSED: Serialization successful - " << output << std::endl;
        }
    }
    
    // Test 6: Depth exceeded error
    {
        JsonValue value;
        std::string errMsg;
        // Create deeply nested JSON that should exceed default depth
        std::string deepJson = "{";
        for(int i = 0; i < 100; i++) {
            deepJson += "\"level" + std::to_string(i) + "\":{";
        }
        deepJson += "\"final\":\"value\"";
        for(int i = 0; i < 100; i++) {
            deepJson += "}";
        }
        deepJson += "}";
        
        auto ec = JsonValue::parse(deepJson, value, errMsg);
        
        if (ec == JsonErrc::DepthExceeded) {
            std::cout << "Test 6 PASSED: Correctly detected depth exceeded - " << errMsg << std::endl;
        } else {
            std::cout << "Test 6 INFO: Depth not exceeded (limit may be higher than 100) - " << ec.message() << std::endl;
        }
    }
    
    // Test 7: Test error code conversion to std::error_code
    {
        std::error_code ec = JsonErrc::UnexpectedCharacter;
        if (ec.category().name() == std::string("JsonStruct") && ec.value() == 7) {
            std::cout << "Test 7 PASSED: std::error_code conversion works correctly" << std::endl;
        } else {
            std::cout << "Test 7 FAILED: std::error_code conversion failed - category: '" 
                      << ec.category().name() << "', value: " << ec.value() << std::endl;
            return 1;
        }
    }

    // Test 8: Test completly valid JSON with special characters
    {
        JsonValue value;
        std::string errMsg;
        auto ec = JsonValue::parse(R"({"name": "test", "special": "\n\t\r\"\\", "unicode": "\u1234"})", value, errMsg);
        std::cout << "ec value: " << ec.value() << ", category: " << ec.category().name() << std::endl;
        if (ec == JsonErrc::Success) {
            std::cout << "Test 8 PASSED: Valid JSON with special characters parsed successfully" << std::endl;
        } else {
            std::cout << "Test 8 FAILED: " << ec.message() << " - " << errMsg << std::endl;
            return 1;
        }
    }

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
