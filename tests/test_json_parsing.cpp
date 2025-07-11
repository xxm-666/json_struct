#include "json_value.h"
#include <iostream>
#include <cassert>

using namespace JsonStruct;

void testJsonParsing() {
    std::cout << "=== JSON Parsing Precision Test ===" << std::endl;
    
    // Test 1: Parse large integers
    std::cout << "Test 1: Parse large integers" << std::endl;
    std::string jsonStr = R"({
        "bigId": 9223372036854775807,
        "mediumId": 9007199254740993,
        "smallId": 42,
        "price": 99.99,
        "discount": 0.15
    })";
    
    auto parsed = JsonValue::parse(jsonStr);
    
    std::cout << "Big ID value: " << parsed["bigId"].toLongLong() << std::endl;
    std::cout << "Big ID type: " << (parsed["bigId"].isInteger() ? "integer" : "double") << std::endl;
    
    std::cout << "Medium ID value: " << parsed["mediumId"].toLongLong() << std::endl;
    std::cout << "Medium ID type: " << (parsed["mediumId"].isInteger() ? "integer" : "double") << std::endl;
    
    std::cout << "Small ID value: " << parsed["smallId"].toLongLong() << std::endl;
    std::cout << "Small ID type: " << (parsed["smallId"].isInteger() ? "integer" : "double") << std::endl;
    
    std::cout << "Price value: " << parsed["price"].toDouble() << std::endl;
    std::cout << "Price type: " << (parsed["price"].isDouble() ? "double" : "integer") << std::endl;
    
    // Verify precision
    assert(parsed["bigId"].toLongLong() == 9223372036854775807LL);
    assert(parsed["mediumId"].toLongLong() == 9007199254740993LL);
    assert(parsed["smallId"].toLongLong() == 42);
    assert(parsed["bigId"].isInteger());
    assert(parsed["mediumId"].isInteger());
    assert(parsed["smallId"].isInteger());
    assert(parsed["price"].isDouble());
    assert(parsed["discount"].isDouble());
    
    std::cout << "âœ?JSON parsing precision test passed" << std::endl;
    
    // Test 2: Serialize and round-trip
    std::cout << "\nTest 2: Serialize and round-trip test" << std::endl;
    
    JsonValue original;
    original["bigInt"] = JsonValue(9223372036854775807LL);
    original["bigIntBeyondSafe"] = JsonValue(9007199254740993LL);
    original["float"] = JsonValue(3.14159);
    
    std::string serialized = original.dump();
    std::cout << "Serialized JSON: " << serialized << std::endl;
    
    auto roundTrip = JsonValue::parse(serialized);
    
    std::cout << "Round-trip big int: " << roundTrip["bigInt"].toLongLong() << std::endl;
    std::cout << "Round-trip beyond safe: " << roundTrip["bigIntBeyondSafe"].toLongLong() << std::endl;
    std::cout << "Round-trip float: " << roundTrip["float"].toDouble() << std::endl;
    
    assert(roundTrip["bigInt"].toLongLong() == 9223372036854775807LL);
    assert(roundTrip["bigIntBeyondSafe"].toLongLong() == 9007199254740993LL);
    assert(std::abs(roundTrip["float"].toDouble() - 3.14159) < 1e-10);
    
    std::cout << "âœ?Serialize and round-trip test passed" << std::endl;
    
    // Test 3: Array of mixed numbers
    std::cout << "\nTest 3: Array of mixed numbers" << std::endl;
    
    std::string arrayJson = R"([9223372036854775807, 3.14159, 42, 9007199254740993])";
    auto arrayParsed = JsonValue::parse(arrayJson);
    
    assert(arrayParsed.isArray());
    assert(arrayParsed[0].isInteger());
    assert(arrayParsed[1].isDouble());
    assert(arrayParsed[2].isInteger());
    assert(arrayParsed[3].isInteger());
    
    assert(arrayParsed[0].toLongLong() == 9223372036854775807LL);
    assert(std::abs(arrayParsed[1].toDouble() - 3.14159) < 1e-10);
    assert(arrayParsed[2].toLongLong() == 42);
    assert(arrayParsed[3].toLongLong() == 9007199254740993LL);
    
    std::cout << "Array element 0: " << arrayParsed[0].toLongLong() << " (integer)" << std::endl;
    std::cout << "Array element 1: " << arrayParsed[1].toDouble() << " (double)" << std::endl;
    std::cout << "Array element 2: " << arrayParsed[2].toLongLong() << " (integer)" << std::endl;
    std::cout << "Array element 3: " << arrayParsed[3].toLongLong() << " (integer)" << std::endl;
    
    std::cout << "âœ?Array of mixed numbers test passed" << std::endl;
}

int main() {
    try {
        testJsonParsing();
        std::cout << "\nðŸŽ‰ All JSON parsing tests passed! Complete precision support verified." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
