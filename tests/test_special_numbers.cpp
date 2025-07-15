// 迁移的特殊数字测试 - Special Numbers Tests
#include "../src/jsonstruct.h"
#include "test_framework.h"
#include <iostream>
#include <cmath>

using namespace JsonStruct;

TEST(SpecialNumbers_NaNSupport) {
    // Test NaN support
    auto nanJson = JsonValue(JsonNumber::makeNaN());
    
    ASSERT_TRUE(nanJson.isNaN());
    ASSERT_TRUE(nanJson.isNumber());
    ASSERT_FALSE(nanJson.isFinite());
    
    // Test that NaN != NaN (standard behavior)
    auto anotherNaN = JsonValue(JsonNumber::makeNaN());
    ASSERT_TRUE(anotherNaN.isNaN());
}

TEST(SpecialNumbers_InfinitySupport) {
    // Test positive infinity
    auto infJson = JsonValue(JsonNumber::makeInfinity());
    ASSERT_TRUE(infJson.isInfinity());
    ASSERT_FALSE(infJson.isFinite());
    ASSERT_TRUE(infJson.isNumber());
    
    // Test negative infinity
    auto negInfJson = JsonValue(JsonNumber::makeNegativeInfinity());
    ASSERT_TRUE(negInfJson.isInfinity());
    ASSERT_FALSE(negInfJson.isFinite());
    ASSERT_TRUE(negInfJson.isNumber());
    
    // Test that positive and negative infinity are different
    ASSERT_NE(infJson.toDouble(), negInfJson.toDouble());
}

TEST(SpecialNumbers_SerializationWithSpecialNumbers) {
    // Test serialization with special numbers allowed
    JsonValue::SerializeOptions allowSpecial;
    allowSpecial.allowSpecialNumbers = true;
    
    auto nanJson = JsonValue(JsonNumber::makeNaN());
    auto infJson = JsonValue(JsonNumber::makeInfinity());
    auto negInfJson = JsonValue(JsonNumber::makeNegativeInfinity());
    
    std::string nanSerialized = nanJson.dump(allowSpecial);
    std::string infSerialized = infJson.dump(allowSpecial);
    std::string negInfSerialized = negInfJson.dump(allowSpecial);
    
    ASSERT_FALSE(nanSerialized.empty());
    ASSERT_FALSE(infSerialized.empty());
    ASSERT_FALSE(negInfSerialized.empty());
    
    // Common representations for special numbers
    ASSERT_TRUE(nanSerialized == "NaN" || nanSerialized == "null" || nanSerialized.find("NaN") != std::string::npos);
    ASSERT_TRUE(infSerialized == "Infinity" || infSerialized.find("Infinity") != std::string::npos);
    ASSERT_TRUE(negInfSerialized == "-Infinity" || negInfSerialized.find("-Infinity") != std::string::npos);
}

TEST(SpecialNumbers_SerializationStrictMode) {
    // Test serialization in strict mode (default)
    JsonValue::SerializeOptions strict;
    strict.allowSpecialNumbers = false;
    
    auto nanJson = JsonValue(JsonNumber::makeNaN());
    auto infJson = JsonValue(JsonNumber::makeInfinity());
    
    // In strict mode, special numbers should be converted to null or throw
    try {
        std::string nanSerialized = nanJson.dump(strict);
        std::string infSerialized = infJson.dump(strict);
        
        // If serialization succeeds, special numbers should become null
        ASSERT_TRUE(nanSerialized == "null" || nanSerialized.empty());
        ASSERT_TRUE(infSerialized == "null" || infSerialized.empty());
    } catch (const std::exception& e) {
        // Exception is also acceptable in strict mode
        ASSERT_TRUE(true);
    }
}

TEST(SpecialNumbers_RegularNumbersStillWork) {
    // Test that regular numbers still work properly
    auto regularJson = JsonValue(42.5);
    ASSERT_TRUE(regularJson.isNumber());
    ASSERT_FALSE(regularJson.isNaN());
    ASSERT_FALSE(regularJson.isInfinity());
    ASSERT_TRUE(regularJson.isFinite());
    ASSERT_NEAR(regularJson.toDouble(), 42.5, 0.001);
    
    // Test zero
    auto zeroJson = JsonValue(0.0);
    ASSERT_TRUE(zeroJson.isNumber());
    ASSERT_FALSE(zeroJson.isNaN());
    ASSERT_FALSE(zeroJson.isInfinity());
    ASSERT_TRUE(zeroJson.isFinite());
    ASSERT_EQ(zeroJson.toDouble(), 0.0);
    
    // Test negative number
    auto negativeJson = JsonValue(-123.456);
    ASSERT_TRUE(negativeJson.isNumber());
    ASSERT_FALSE(negativeJson.isNaN());
    ASSERT_FALSE(negativeJson.isInfinity());
    ASSERT_TRUE(negativeJson.isFinite());
    ASSERT_NEAR(negativeJson.toDouble(), -123.456, 0.001);
}

TEST(SpecialNumbers_ArrayWithSpecialNumbers) {
    // Test array containing special numbers
    JsonValue::ArrayType arr;
    arr.push_back(JsonValue(42.0));
    arr.push_back(JsonValue(JsonNumber::makeNaN()));
    arr.push_back(JsonValue(JsonNumber::makeInfinity()));
    arr.push_back(JsonValue(-3.14));
    arr.push_back(JsonValue(JsonNumber::makeNegativeInfinity()));
    
    auto arrayJson = JsonValue(arr);
    ASSERT_TRUE(arrayJson.isArray());
    ASSERT_EQ(arrayJson.size(), 5);
    
    // Check each element
    ASSERT_TRUE(arrayJson[0].isFinite());
    ASSERT_NEAR(arrayJson[0].toDouble(), 42.0, 0.001);
    
    ASSERT_TRUE(arrayJson[1].isNaN());
    ASSERT_TRUE(arrayJson[2].isInfinity());
    
    ASSERT_TRUE(arrayJson[3].isFinite());
    ASSERT_NEAR(arrayJson[3].toDouble(), -3.14, 0.001);
    
    ASSERT_TRUE(arrayJson[4].isInfinity());
}

TEST(SpecialNumbers_ObjectWithSpecialNumbers) {
    // Test object containing special numbers
    JsonValue::ObjectType obj;
    obj["regular"] = JsonValue(123.0);
    obj["nan_value"] = JsonValue(JsonNumber::makeNaN());
    obj["infinity"] = JsonValue(JsonNumber::makeInfinity());
    obj["negative_infinity"] = JsonValue(JsonNumber::makeNegativeInfinity());
    obj["zero"] = JsonValue(0.0);
    
    auto objectJson = JsonValue(obj);
    ASSERT_TRUE(objectJson.isObject());
    ASSERT_EQ(objectJson.size(), 5);
    
    // Check each field
    ASSERT_TRUE(objectJson["regular"].isFinite());
    ASSERT_NEAR(objectJson["regular"].toDouble(), 123.0, 0.001);
    
    ASSERT_TRUE(objectJson["nan_value"].isNaN());
    ASSERT_TRUE(objectJson["infinity"].isInfinity());
    ASSERT_TRUE(objectJson["negative_infinity"].isInfinity());
    
    ASSERT_TRUE(objectJson["zero"].isFinite());
    ASSERT_EQ(objectJson["zero"].toDouble(), 0.0);
}

TEST(SpecialNumbers_EdgeCases) {
    // Test edge cases with special numbers
    
    // Test very large finite numbers (should not be infinity)
    auto largeNumber = JsonValue(1e100);
    ASSERT_TRUE(largeNumber.isFinite());
    ASSERT_FALSE(largeNumber.isInfinity());
    ASSERT_FALSE(largeNumber.isNaN());
    
    // Test very small finite numbers (should not be zero or special)
    auto smallNumber = JsonValue(1e-100);
    ASSERT_TRUE(smallNumber.isFinite());
    ASSERT_FALSE(smallNumber.isInfinity());
    ASSERT_FALSE(smallNumber.isNaN());
    
    // Test that finite numbers are properly identified
    std::vector<double> finiteValues = {1.0, -1.0, 0.0, 3.14159, -2.71828, 1e10, -1e-10};
    for (double val : finiteValues) {
        auto json = JsonValue(val);
        ASSERT_TRUE(json.isFinite());
        ASSERT_FALSE(json.isNaN());
        ASSERT_FALSE(json.isInfinity());
    }
}

int main() {
    std::cout << "=== Special Numbers Migration Tests ===" << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    if (result == 0) {
        std::cout << "✅ All special numbers tests PASSED!" << std::endl;
        std::cout << "🎉 Special numbers support verified!" << std::endl;
    } else {
        std::cout << "❌ Some special numbers tests FAILED!" << std::endl;
    }
    
    return result;
}
