#include "jsonstruct.h"
#include <iostream>
#include <cassert>
#include <climits>

using namespace JsonStruct;

void testPrecisionPreservation() {
    std::cout << "=== JsonValue Precision Test ===" << std::endl;
    
    // Test 1: Large integer precision preservation
    std::cout << "Test 1: Large integer precision preservation" << std::endl;
    long long bigInt = 9223372036854775807LL;  // LLONG_MAX
    JsonValue json1(bigInt);
    
    std::cout << "Original value: " << bigInt << std::endl;
    std::cout << "JSON type: " << (json1.isInteger() ? "integer" : "double") << std::endl;
    std::cout << "Retrieved value: " << json1.toLongLong() << std::endl;
    std::cout << "Precision preserved: " << (json1.toLongLong() == bigInt ? "yes" : "no") << std::endl;
    assert(json1.isInteger());
    assert(json1.toLongLong() == bigInt);
    std::cout << "✅ Large integer precision test passed\n" << std::endl;
    
    // Test 2: IEEE 754 safe boundary test
    std::cout << "Test 2: IEEE 754 safe boundary test" << std::endl;
    long long boundary = JsonNumber::SAFE_INTEGER_MAX;
    long long beyondBoundary = boundary + 1;
    
    JsonValue json2(boundary);
    JsonValue json3(beyondBoundary);
    
    std::cout << "Safe boundary value (2^53): " << boundary << std::endl;
    std::cout << "Boundary value type: " << (json2.isInteger() ? "integer" : "double") << std::endl;
    std::cout << "Boundary value retrieved: " << json2.toLongLong() << std::endl;
    std::cout << "Boundary precision: " << (json2.toLongLong() == boundary ? "preserved" : "lost") << std::endl;
    
    std::cout << "Beyond boundary value (2^53+1): " << beyondBoundary << std::endl;
    std::cout << "Beyond boundary type: " << (json3.isInteger() ? "integer" : "double") << std::endl;
    std::cout << "Beyond boundary retrieved: " << json3.toLongLong() << std::endl;
    std::cout << "Beyond boundary precision: " << (json3.toLongLong() == beyondBoundary ? "preserved" : "lost") << std::endl;
    
    assert(json2.toLongLong() == boundary);
    assert(json3.toLongLong() == beyondBoundary);
    std::cout << "✅ Boundary value test passed\n" << std::endl;
    
    // Test 3: Double precision test
    std::cout << "Test 3: Double precision test" << std::endl;
    double precise = 3.141592653589793;
    JsonValue json4(precise);
    
    std::cout << "Original double: " << precise << std::endl;
    std::cout << "JSON type: " << (json4.isDouble() ? "double" : "integer") << std::endl;
    std::cout << "Retrieved value: " << json4.toDouble() << std::endl;
    std::cout << "Precision error: " << std::abs(json4.toDouble() - precise) << std::endl;
    
    assert(json4.isDouble());
    assert(std::abs(json4.toDouble() - precise) < 1e-15);
    std::cout << "✅ Double precision test passed\n" << std::endl;
    
    // Test 4: Type distinction test
    std::cout << "Test 4: Integer vs double type distinction" << std::endl;
    JsonValue intJson(42LL);
    JsonValue floatJson(42.0);
    JsonValue floatJson2(42.5);
    
    std::cout << "Integer 42: " << (intJson.isInteger() ? "integer type" : "double type") << std::endl;
    std::cout << "Double 42.0: " << (floatJson.isDouble() ? "double type" : "integer type") << std::endl;
    std::cout << "Double 42.5: " << (floatJson2.isDouble() ? "double type" : "integer type") << std::endl;
    
    assert(intJson.isInteger());
    assert(floatJson.isDouble());
    assert(floatJson2.isDouble());
    std::cout << "✅ Type distinction test passed\n" << std::endl;
    
    // Test 5: Safe access interface test
    std::cout << "Test 5: Safe access interface test" << std::endl;
    JsonValue bigIntJson(9223372036854775807LL);
    
    auto intOpt = bigIntJson.getInteger();
    auto doubleOpt = bigIntJson.getNumber();
    
    std::cout << "Safe integer access: " << (intOpt.has_value() ? "success" : "failed") << std::endl;
    if (intOpt.has_value()) {
        std::cout << "Retrieved integer value: " << *intOpt << std::endl;
    }
    
    std::cout << "Safe double access: " << (doubleOpt.has_value() ? "success" : "failed") << std::endl;
    if (doubleOpt.has_value()) {
        std::cout << "Retrieved double value: " << *doubleOpt << std::endl;
    }
    
    assert(intOpt.has_value());
    assert(doubleOpt.has_value());
    assert(*intOpt == 9223372036854775807LL);
    std::cout << "✅ Safe access test passed\n" << std::endl;
    
    // Test 6: Precision loss comparison
    std::cout << "Test 6: Precision loss comparison with old implementation" << std::endl;
    long long testValue = 9007199254740993LL;  // 2^53 + 1, loses precision in double
    
    // New implementation
    JsonValue newImpl(testValue);
    
    // Simulate old implementation
    double oldImpl = static_cast<double>(testValue);
    long long oldRetrieved = static_cast<long long>(oldImpl);
    
    std::cout << "Test value: " << testValue << std::endl;
    std::cout << "New implementation retrieved: " << newImpl.toLongLong() << std::endl;
    std::cout << "Old implementation retrieved: " << oldRetrieved << std::endl;
    std::cout << "New implementation precision: " << (newImpl.toLongLong() == testValue ? "preserved" : "lost") << std::endl;
    std::cout << "Old implementation precision: " << (oldRetrieved == testValue ? "preserved" : "lost") << std::endl;
    
    assert(newImpl.toLongLong() == testValue);
    assert(oldRetrieved != testValue);  // Old implementation indeed loses precision
    std::cout << "✅ Precision comparison test passed" << std::endl;
}

void testCompatibility() {
    std::cout << "\n=== Backward Compatibility Test ===" << std::endl;
    
    // Test that old APIs still work
    JsonValue json(123.45);
    
    // These calls should still work
    double d = json.toDouble();
    int i = json.toInt();
    long long ll = json.toLongLong();
    
    std::cout << "Original value: 123.45" << std::endl;
    std::cout << "toDouble(): " << d << std::endl;
    std::cout << "toInt(): " << i << std::endl;
    std::cout << "toLongLong(): " << ll << std::endl;
    
    assert(std::abs(d - 123.45) < 1e-10);
    assert(i == 123);
    assert(ll == 123);
    std::cout << "✅ Backward compatibility test passed" << std::endl;
}

int main() {
    try {
        testPrecisionPreservation();
        testCompatibility();
        
        std::cout << "\n🎉 All tests passed! Precision issues resolved." << std::endl;
        
        std::cout << "\n=== Performance and Memory Usage Analysis ===" << std::endl;
        std::cout << "JsonNumber size: " << sizeof(JsonNumber) << " bytes" << std::endl;
        std::cout << "double size: " << sizeof(double) << " bytes" << std::endl;
        std::cout << "Memory increase: " << (sizeof(JsonNumber) - sizeof(double)) << " bytes per number" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
