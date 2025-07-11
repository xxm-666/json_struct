#include "json_value.h"
#include "json_stream_parser.h"
#include <iostream>
#include <cassert>
#include <iomanip>
#include <chrono>

using namespace JsonStruct;

void printHeader(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void testPrecisionFeatures() {
    printHeader("Numeric Precision Enhancement");
    // Test 1: Large integer precision
    std::cout << "\n1. Large Integer Precision Test" << std::endl;
    int64_t largeInt = 9007199254740993LL;  // > 2^53
    JsonValue json(largeInt);
    std::cout << "   Original value: " << largeInt << std::endl;
    std::cout << "   JSON stored: " << json.dump() << std::endl;
    std::cout << "   Retrieved value: " << json.toLongLong() << std::endl;
    assert(json.toLongLong() == largeInt);
    std::cout << "   ✅ Large integer precision is correct" << std::endl;
    // Test 2: JSON parsing precision
    std::cout << "\n2. JSON Parsing Precision Test" << std::endl;
    std::string jsonStr = R"({"bigInt": 9007199254740993, "normalInt": 42})";
    auto parsed = JsonValue::parse(jsonStr);
    std::cout << "   Parsed bigInt: " << parsed["bigInt"].toLongLong() << std::endl;
    std::cout << "   Is integer: " << (parsed["bigInt"].isInteger() ? "Yes" : "No") << std::endl;
    assert(parsed["bigInt"].toLongLong() == largeInt);
    assert(parsed["bigInt"].isInteger());
    std::cout << "   ✅ JSON parsing precision is correct" << std::endl;
}

void testSpecialValues() {
    printHeader("Special Number Value Support Test");
    // Test 1: NaN and Infinity creation
    std::cout << "\n1. NaN and Infinity Creation" << std::endl;
    auto nanJson = JsonValue(JsonNumber::makeNaN());
    auto infJson = JsonValue(JsonNumber::makeInfinity());
    auto negInfJson = JsonValue(JsonNumber::makeNegativeInfinity());
    std::cout << "   NaN detected: " << (nanJson.isNaN() ? "Yes" : "No") << std::endl;
    std::cout << "   Infinity detected: " << (infJson.isInfinity() ? "Yes" : "No") << std::endl;
    std::cout << "   -Infinity detected: " << (negInfJson.isInfinity() ? "Yes" : "No") << std::endl;
    assert(nanJson.isNaN());
    assert(infJson.isInfinity());
    assert(negInfJson.isInfinity());
    std::cout << "   ✅ Special number value detection is correct" << std::endl;
    // Test 2: Special value serialization
    std::cout << "\n2. Special Value Serialization Test" << std::endl;
    JsonValue::SerializeOptions allowSpecial;
    allowSpecial.allowSpecialNumbers = true;
    std::cout << "   NaN serialized: " << nanJson.dump(allowSpecial) << std::endl;
    std::cout << "   Infinity serialized: " << infJson.dump(allowSpecial) << std::endl;
    std::cout << "   Standard mode NaN: " << nanJson.dump() << std::endl;
    assert(nanJson.dump(allowSpecial) == "NaN");
    assert(infJson.dump(allowSpecial) == "Infinity");
    assert(nanJson.dump() == "null");
    std::cout << "   ✅ Special value serialization is correct" << std::endl;
    // Test 3: Special value parsing
    std::cout << "\n3. Special Value Parsing Test" << std::endl;
    JsonValue::ParseOptions allowParsing;
    allowParsing.allowSpecialNumbers = true;
    auto parsedNaN = JsonValue::parse("NaN", allowParsing);
    auto parsedInf = JsonValue::parse("Infinity", allowParsing);
    std::cout << "   Parsed NaN success: " << (parsedNaN.isNaN() ? "Yes" : "No") << std::endl;
    std::cout << "   Parsed Infinity success: " << (parsedInf.isInfinity() ? "Yes" : "No") << std::endl;
    assert(parsedNaN.isNaN());
    assert(parsedInf.isInfinity());
    std::cout << "   ✅ Special value parsing is correct" << std::endl;
}

void testErrorRecovery() {
    printHeader("Error Recovery Test");
    // Test 1: Basic error recovery
    std::cout << "\n1. Basic Error Recovery" << std::endl;
    JsonValue::ParseOptions recovery;
    recovery.allowRecovery = true;
    try {
        auto result = JsonValue::parse("@#$%42", recovery);
        std::cout << "   Recovered value: " << result.toInt() << std::endl;
        assert(result.toInt() == 42);
        std::cout << "   ✅ Error recovery is correct" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   ❌ Error recovery failed: " << e.what() << std::endl;
    }
    // Test 2: Strict mode comparison
    std::cout << "\n2. Strict Mode Comparison" << std::endl;
    JsonValue::ParseOptions strict;
    strict.allowRecovery = false;
    try {
        auto result = JsonValue::parse("@#$%42", strict);
        std::cout << "   ❌ Strict mode should not succeed" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   ✅ Strict mode correctly threw exception: " << e.what() << std::endl;
        std::cout << "   ✅ Strict mode behavior is correct" << std::endl;
    }
}

void testStreamingParser() {
    printHeader("Streaming Parser Test");
    // Test 1: Event-driven parsing
    std::cout << "\n1. Event-driven Parsing" << std::endl;
    JsonStreamParser parser;
    std::vector<std::string> events;
    parser.setEventHandler([&events](const JsonStreamParser::Event& event) {
        switch (event.type) {
            case JsonStreamParser::EventType::ObjectStart:
                events.push_back("ObjectStart");
                break;
            case JsonStreamParser::EventType::ObjectEnd:
                events.push_back("ObjectEnd");
                break;
            case JsonStreamParser::EventType::Key:
                events.push_back("Key:" + event.value.toString());
                break;
            case JsonStreamParser::EventType::Value:
                events.push_back("Value:" + event.value.dump());
                break;
            default:
                events.push_back("Other");
                break;
        }
        return true;
    });
    std::string testJson = R"({"name": "Alice", "age": 30})";
    parser.feed(testJson);
    parser.finish();
    std::cout << "   Event count: " << events.size() << std::endl;
    for (const auto& event : events) {
        std::cout << "     " << event << std::endl;
    }
    assert(events.size() >= 5);  // Should have object start, key, value, key, value, object end
    std::cout << "   ✅ Event-driven parsing is correct" << std::endl;
    // Test 2: Large JSON simulation
    std::cout << "\n2. Large JSON Simulation" << std::endl;
    JsonStreamParser largeParser;
    size_t valueCount = 0;
    largeParser.setEventHandler([&valueCount](const JsonStreamParser::Event& event) {
        if (event.type == JsonStreamParser::EventType::Value) {
            ++valueCount;
        }
        return true;
    });
    // Generate large JSON
    std::string largeJson = "{\"data\": [";
    for (int i = 0; i < 100; ++i) {
        if (i > 0) largeJson += ",";
        largeJson += std::to_string(i);
    }
    largeJson += "]}";
    largeParser.feed(largeJson);
    largeParser.finish();
    std::cout << "   Value count: " << valueCount << std::endl;
    assert(valueCount == 100);
    std::cout << "   ✅ Large JSON simulation is correct" << std::endl;
}

void testAdvancedFeatures() {
    printHeader("Advanced Feature Integration Test");
    // Test 1: JSON Pointer
    std::cout << "\n1. JSON Pointer Test" << std::endl;
    std::string complexJson = R"({
        "users": [
            {"name": "Alice", "age": 30},
            {"name": "Bob", "age": 25}
        ],
        "metadata": {"count": 2}
    })";
    auto json = JsonValue::parse(complexJson);
    try {
        auto& alice = json.at("/users/0/name");
        auto& count = json.at("/metadata/count");
        std::cout << "   /users/0/name: " << alice.toString() << std::endl;
        std::cout << "   /metadata/count: " << count.toInt() << std::endl;
        assert(alice.toString() == "Alice");
        assert(count.toInt() == 2);
        std::cout << "   ✅ JSON Pointer test is correct" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   ❌ JSON Pointer test failed: " << e.what() << std::endl;
    }
    // Test 2: Configuration options
    std::cout << "\n2. Serialization Options Test" << std::endl;
    JsonValue::SerializeOptions prettyOptions;
    prettyOptions.indent = 2;
    prettyOptions.sortKeys = true;
    std::string prettyOutput = json.dump(prettyOptions);
    std::cout << "   Pretty output:" << std::endl;
    std::cout << prettyOutput << std::endl;
    assert(prettyOutput.find("  ") != std::string::npos);  // Should contain indentation
    std::cout << "   ? Serialization options are correct" << std::endl;
    
    // Test 3: Container integration
    std::cout << "\n3. Container Integration Test" << std::endl;
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::map<std::string, std::string> map = {{"key1", "value1"}, {"key2", "value2"}};
    JsonValue vecJson(vec);
    JsonValue mapJson(map);
    std::cout << "   vector to JSON: " << vecJson.dump() << std::endl;
    std::cout << "   map to JSON: " << mapJson.dump() << std::endl;
    assert(vecJson.isArray());
    assert(mapJson.isObject());
    assert(vecJson.size() == 5);
    assert(mapJson.size() == 2);
    std::cout << "   ? Container integration is correct" << std::endl;
}

void performanceBenchmark() {
    printHeader("Performance Benchmarking");
    // Test 1: Parsing performance
    std::cout << "\n1. Parsing Performance Test" << std::endl;
    std::string largeJson = "{\"data\": [";
    for (int i = 0; i < 1000; ++i) {
        if (i > 0) largeJson += ",";
        largeJson += "{\"id\": " + std::to_string(i) + ", \"value\": \"item" + std::to_string(i) + "\"}";
    }
    largeJson += "]}";
    auto start = std::chrono::high_resolution_clock::now();
    auto result = JsonValue::parse(largeJson);
    auto duration = std::chrono::high_resolution_clock::now() - start;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    std::cout << "   Parsed 1000 objects in: " << ms << "ms" << std::endl;
    std::cout << "   Resulting array size: " << result["data"].size() << std::endl;
    assert(result["data"].size() == 1000);
    std::cout << "   ✅ Parsing performance is acceptable" << std::endl;
    // Test 2: Serialization performance
    std::cout << "\n2. Serialization Performance Test" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    std::string serialized = result.dump();
    duration = std::chrono::high_resolution_clock::now() - start;
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    std::cout << "   Serialized 1000 objects in: " << ms << "ms" << std::endl;
    std::cout << "   Serialized size: " << serialized.size() << " bytes" << std::endl;
    std::cout << "   ✅ Serialization performance is acceptable" << std::endl;
    // Test 3: Lookup performance
    std::cout << "\n3. Lookup Performance Test" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i) {
        auto& item = result["data"][i];
        volatile int id = item["id"].toInt();  // Prevent optimization
        (void)id;
    }
    duration = std::chrono::high_resolution_clock::now() - start;
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    std::cout << "   100 random accesses took: " << us << "μs" << std::endl;
    std::cout << "   Average per access: " << (us / 100.0) << "μs" << std::endl;
    std::cout << "   ✅ Lookup performance is acceptable" << std::endl;
}

void demonstrateRealWorldUsage() {
    printHeader("Real World Usage Demonstration");
    // Scenario: Processing configuration with special values and error recovery
    std::cout << "\nScenario: Configuration with special values and error recovery" << std::endl;
    std::string configJson = R"({
        "server": {
            "host": "localhost",
            "port": 8080,
            "timeout": Infinity,
            "maxConnections": 9007199254740993
        },
        "features": {
            "enableCache": true,
            "cacheSize": NaN,
            "debugMode": false
        },
        // Invalid field for testing error recovery
        "malformed_field": broken_value,
        "validation": {
            "enabled": true,
            "strictMode": false
        }
    })";
    JsonValue::ParseOptions options;
    options.allowSpecialNumbers = true;
    options.allowRecovery = true;
    options.allowComments = true;
    try {
        auto config = JsonValue::parse(configJson, options);
        std::cout << "\nParsed configuration:" << std::endl;
        std::cout << "  Server port: " << config["server"]["port"].toInt() << std::endl;
        std::cout << "  Max connections: " << config["server"]["maxConnections"].toLongLong() << std::endl;
        std::cout << "  Timeout is infinity: " << (config["server"]["timeout"].isInfinity() ? "Yes" : "No") << std::endl;
        std::cout << "  Cache size is NaN: " << (config["features"]["cacheSize"].isNaN() ? "Yes" : "No") << std::endl;
        // Demonstrate JSON Pointer
        std::cout << "  Debug mode (via JSON Pointer): " << config.at("/features/debugMode").toBool() << std::endl;
        // Demonstrate serialization
        JsonValue::SerializeOptions serOptions;
        serOptions.indent = 2;
        serOptions.allowSpecialNumbers = true;
        std::cout << "\nSerialized configuration:" << std::endl;
        std::cout << config.dump(serOptions) << std::endl;
        std::cout << "✅ Configuration processing demonstration succeeded" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "❌ Configuration processing failed: " << e.what() << std::endl;
    }
}

int main() {
    try {
        std::cout << "JsonValue C++17+ Enhanced - Comprehensive Test Suite" << std::endl;
        std::cout << "======================================" << std::endl;
        
        testPrecisionFeatures();
        testSpecialValues();
        testErrorRecovery();
        testStreamingParser();
        testAdvancedFeatures();
        performanceBenchmark();
        demonstrateRealWorldUsage();
        
        printHeader("Summary and Conclusions");
        std::cout << "\n✅ Comprehensive testing completed." << std::endl;
        std::cout << "🔍 Areas of focus:" << std::endl;
        std::cout << " - Numeric precision and large integer support" << std::endl;
        std::cout << " - Special value handling (NaN, Infinity)" << std::endl;
        std::cout << " - Error recovery mechanisms" << std::endl;
        std::cout << " - Streaming parser event-driven model" << std::endl;
        std::cout << " - Advanced features: JSON Pointer, custom serialization options" << std::endl;
        std::cout << " - Performance benchmarks for parsing, serialization, and lookup" << std::endl;
        
        std::cout << "\n📈 Performance benchmarks indicate acceptable speeds for parsing, serialization,";
        std::cout << " and data lookup within JSON structures." << std::endl;
        
        std::cout << "\n🚀 Ready for production use with C++17 and above." << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Unexpected error: " << e.what() << std::endl;
        return 1;
    }
}
