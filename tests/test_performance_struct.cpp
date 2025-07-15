// 迁移的性能结构体测试 - Performance Struct Tests
#include "../src/jsonstruct.h"
#include "test_framework.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace JsonStruct;

struct PerformanceStruct {
    int id;
    std::string name;
    double value;
    std::vector<int> data;
    std::map<std::string, std::string> properties;
    
    JSON_AUTO(id, name, value, data, properties)
};

TEST(PerformanceStruct_BasicSerialization) {
    // Create test object
    PerformanceStruct obj;
    obj.id = 123;
    obj.name = "test_object";
    obj.value = 45.67;
    obj.data = {10, 20, 30};
    obj.properties = {{"key1", "value1"}, {"key2", "value2"}};
    
    // Test serialization
    auto jsonVal = obj.toJson();
    ASSERT_TRUE(jsonVal.isObject());
    
    auto jsonObj = jsonVal.toObject();
    ASSERT_EQ(jsonObj.size(), 5); // 5 fields: id, name, value, data, properties
    
    // Verify each field
    ASSERT_TRUE(jsonObj.find("id") != jsonObj.end());
    ASSERT_TRUE(jsonObj.find("name") != jsonObj.end());
    ASSERT_TRUE(jsonObj.find("value") != jsonObj.end());
    ASSERT_TRUE(jsonObj.find("data") != jsonObj.end());
    ASSERT_TRUE(jsonObj.find("properties") != jsonObj.end());
    
    // Verify field values
    ASSERT_EQ(jsonObj["id"].toInt(), 123);
    ASSERT_EQ(jsonObj["name"].toString(), "test_object");
    ASSERT_NEAR(jsonObj["value"].toDouble(), 45.67, 0.001);
    ASSERT_TRUE(jsonObj["data"].isArray());
    ASSERT_TRUE(jsonObj["properties"].isObject());
}

TEST(PerformanceStruct_ComplexDataSerialization) {
    PerformanceStruct obj;
    obj.id = 456;
    obj.name = "complex_object";
    obj.value = 123.456;
    obj.data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    obj.properties = {
        {"type", "test"},
        {"category", "performance"},
        {"environment", "debug"},
        {"version", "1.0.0"}
    };
    
    // Test serialization
    auto jsonVal = obj.toJson();
    ASSERT_TRUE(jsonVal.isObject());
    
    // Test array serialization
    ASSERT_TRUE(jsonVal["data"].isArray());
    ASSERT_EQ(jsonVal["data"].size(), 10);
    for (size_t i = 0; i < 10; ++i) {
        ASSERT_EQ(jsonVal["data"][i].toInt(), static_cast<int>(i + 1));
    }
    
    // Test map serialization
    ASSERT_TRUE(jsonVal["properties"].isObject());
    auto props = jsonVal["properties"].toObject();
    ASSERT_EQ(props.size(), 4);
    ASSERT_EQ(props["type"].toString(), "test");
    ASSERT_EQ(props["category"].toString(), "performance");
    ASSERT_EQ(props["environment"].toString(), "debug");
    ASSERT_EQ(props["version"].toString(), "1.0.0");
}

TEST(PerformanceStruct_Deserialization) {
    // Create original object
    PerformanceStruct original;
    original.id = 789;
    original.name = "deserialize_test";
    original.value = 987.654;
    original.data = {100, 200, 300};
    original.properties = {{"key1", "value1"}, {"key2", "value2"}};
    
    // Serialize
    auto jsonVal = original.toJson();
    std::string jsonStr = jsonVal.dump();
    
    // Parse JSON string
    auto parsed = JsonValue::parse(jsonStr);
    
    // Deserialize
    PerformanceStruct restored;
    restored.fromJson(parsed);
    
    // Verify deserialization
    ASSERT_EQ(restored.id, original.id);
    ASSERT_EQ(restored.name, original.name);
    ASSERT_NEAR(restored.value, original.value, 0.001);
    ASSERT_EQ(restored.data.size(), original.data.size());
    ASSERT_EQ(restored.properties.size(), original.properties.size());
    
    // Verify array data
    for (size_t i = 0; i < restored.data.size(); ++i) {
        ASSERT_EQ(restored.data[i], original.data[i]);
    }
    
    // Verify map data
    for (const auto& pair : original.properties) {
        ASSERT_TRUE(restored.properties.find(pair.first) != restored.properties.end());
        ASSERT_EQ(restored.properties[pair.first], pair.second);
    }
}

TEST(PerformanceStruct_EdgeCases) {
    // Test empty data
    PerformanceStruct emptyObj;
    emptyObj.id = 0;
    emptyObj.name = "";
    emptyObj.value = 0.0;
    emptyObj.data = {};
    emptyObj.properties = {};
    
    auto jsonVal = emptyObj.toJson();
    ASSERT_TRUE(jsonVal.isObject());
    ASSERT_EQ(jsonVal["id"].toInt(), 0);
    ASSERT_EQ(jsonVal["name"].toString(), "");
    ASSERT_EQ(jsonVal["value"].toDouble(), 0.0);
    ASSERT_TRUE(jsonVal["data"].isArray());
    ASSERT_EQ(jsonVal["data"].size(), 0);
    ASSERT_TRUE(jsonVal["properties"].isObject());
    ASSERT_EQ(jsonVal["properties"].size(), 0);
    
    // Test deserialization of empty data
    PerformanceStruct restored;
    restored.fromJson(jsonVal);
    ASSERT_EQ(restored.id, 0);
    ASSERT_EQ(restored.name, "");
    ASSERT_EQ(restored.value, 0.0);
    ASSERT_EQ(restored.data.size(), 0);
    ASSERT_EQ(restored.properties.size(), 0);
}

TEST(PerformanceStruct_LargeData) {
    // Test with larger datasets
    PerformanceStruct largeObj;
    largeObj.id = 99999;
    largeObj.name = "large_performance_test_object_with_very_long_name";
    largeObj.value = 123456.789123;
    
    // Large array
    for (int i = 0; i < 100; ++i) {
        largeObj.data.push_back(i * i);
    }
    
    // Large map
    for (int i = 0; i < 50; ++i) {
        largeObj.properties["key" + std::to_string(i)] = "value" + std::to_string(i);
    }
    
    // Test serialization
    auto jsonVal = largeObj.toJson();
    ASSERT_TRUE(jsonVal.isObject());
    ASSERT_EQ(jsonVal["data"].size(), 100);
    ASSERT_EQ(jsonVal["properties"].size(), 50);
    
    // Test string output
    std::string jsonStr = jsonVal.dump();
    ASSERT_FALSE(jsonStr.empty());
    
    // Test round-trip
    auto parsed = JsonValue::parse(jsonStr);
    PerformanceStruct restored;
    restored.fromJson(parsed);
    
    ASSERT_EQ(restored.id, largeObj.id);
    ASSERT_EQ(restored.name, largeObj.name);
    ASSERT_NEAR(restored.value, largeObj.value, 0.001);
    ASSERT_EQ(restored.data.size(), largeObj.data.size());
    ASSERT_EQ(restored.properties.size(), largeObj.properties.size());
}

int main() {
    std::cout << "=== Performance Struct Migration Tests ===" << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    if (result == 0) {
        std::cout << "✅ All performance struct tests PASSED!" << std::endl;
        std::cout << "🎉 Performance validation complete!" << std::endl;
    } else {
        std::cout << "❌ Some performance struct tests FAILED!" << std::endl;
    }
    
    return result;
}
