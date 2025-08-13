// 迁移的JSON解析测试 - JSON Parsing Precision Tests
#include "../src/jsonstruct.h"
#include <test_framework/test_framework.h>
#include <iostream>
#include <cmath>

using namespace JsonStruct;

TEST(JsonParsing_LargeIntegers) {
    // Test parsing large integers
    std::string jsonStr = R"({
        "bigId": 9223372036854775807,
        "mediumId": 9007199254740993,
        "smallId": 42,
        "price": 99.99,
        "discount": 0.15
    })";
    
    auto parsed = JsonValue::parse(jsonStr);
    
    // Verify values and types
    ASSERT_EQ(parsed["bigId"].toLongLong(), 9223372036854775807LL);
    ASSERT_EQ(parsed["mediumId"].toLongLong(), 9007199254740993LL);
    ASSERT_EQ(parsed["smallId"].toLongLong(), 42);
    
    // Verify types
    ASSERT_TRUE(parsed["bigId"].isInteger());
    ASSERT_TRUE(parsed["mediumId"].isInteger());
    ASSERT_TRUE(parsed["smallId"].isInteger());
    ASSERT_TRUE(parsed["price"].isDouble());
    ASSERT_TRUE(parsed["discount"].isDouble());
    
    // Verify floating point values
    ASSERT_NEAR(parsed["price"].toDouble(), 99.99, 0.001);
    ASSERT_NEAR(parsed["discount"].toDouble(), 0.15, 0.001);
}

TEST(JsonParsing_SerializeRoundtrip) {
    // Test serialize and round-trip
    JsonValue original;
    original["bigInt"] = JsonValue(9223372036854775807LL);
    original["bigIntBeyondSafe"] = JsonValue(9007199254740993LL);
    original["float"] = JsonValue(3.14159);
    
    std::string serialized = original.dump();
    ASSERT_FALSE(serialized.empty());
    
    auto roundTrip = JsonValue::parse(serialized);
    
    // Verify round-trip precision
    ASSERT_EQ(roundTrip["bigInt"].toLongLong(), 9223372036854775807LL);
    ASSERT_EQ(roundTrip["bigIntBeyondSafe"].toLongLong(), 9007199254740993LL);
    ASSERT_NEAR(roundTrip["float"].toDouble(), 3.14159, 1e-10);
    
    // Verify types are preserved
    ASSERT_TRUE(roundTrip["bigInt"].isInteger());
    ASSERT_TRUE(roundTrip["bigIntBeyondSafe"].isInteger());
    ASSERT_TRUE(roundTrip["float"].isDouble());
}

TEST(JsonParsing_ArrayMixedNumbers) {
    // Test array of mixed numbers
    std::string arrayJson = R"([9223372036854775807, 3.14159, 42, 9007199254740993])";
    auto arrayParsed = JsonValue::parse(arrayJson);
    
    ASSERT_TRUE(arrayParsed.isArray());
    ASSERT_EQ(arrayParsed.size(), 4);
    
    // Verify types
    ASSERT_TRUE(arrayParsed[0].isInteger());
    ASSERT_TRUE(arrayParsed[1].isDouble());
    ASSERT_TRUE(arrayParsed[2].isInteger());
    ASSERT_TRUE(arrayParsed[3].isInteger());
    
    // Verify values
    ASSERT_EQ(arrayParsed[0].toLongLong(), 9223372036854775807LL);
    ASSERT_NEAR(arrayParsed[1].toDouble(), 3.14159, 1e-10);
    ASSERT_EQ(arrayParsed[2].toLongLong(), 42);
    ASSERT_EQ(arrayParsed[3].toLongLong(), 9007199254740993LL);
}

TEST(JsonParsing_EdgeCases) {
    // Test edge cases for number parsing
    
    // Test zero
    auto zeroJson = JsonValue::parse("0");
    ASSERT_TRUE(zeroJson.isInteger());
    ASSERT_EQ(zeroJson.toLongLong(), 0);
    
    // Test negative numbers
    auto negativeJson = JsonValue::parse("-123456789");
    ASSERT_TRUE(negativeJson.isInteger());
    ASSERT_EQ(negativeJson.toLongLong(), -123456789);
    
    // Test scientific notation
    auto scientificJson = JsonValue::parse("1.23e10");
    ASSERT_TRUE(scientificJson.isDouble());
    ASSERT_NEAR(scientificJson.toDouble(), 1.23e10, 1e6);
    
    // Test very small decimal
    auto smallDecimalJson = JsonValue::parse("0.000001");
    ASSERT_TRUE(smallDecimalJson.isDouble());
    ASSERT_NEAR(smallDecimalJson.toDouble(), 0.000001, 1e-10);
}

TEST(JsonParsing_ComplexObjects) {
    // Test complex nested objects with different number types
    std::string complexJson = R"({
        "user": {
            "id": 9223372036854775807,
            "balance": 1234.56,
            "scores": [100, 95.5, 87, 92.3]
        },
        "metadata": {
            "timestamp": 1634567890123,
            "version": 2.1,
            "ratios": [0.1, 0.25, 0.5, 0.75, 1.0]
        }
    })";
    
    auto parsed = JsonValue::parse(complexJson);
    
    // Verify nested structure
    ASSERT_TRUE(parsed.isObject());
    ASSERT_TRUE(parsed["user"].isObject());
    ASSERT_TRUE(parsed["metadata"].isObject());
    
    // Verify user data
    ASSERT_EQ(parsed["user"]["id"].toLongLong(), 9223372036854775807LL);
    ASSERT_TRUE(parsed["user"]["id"].isInteger());
    ASSERT_NEAR(parsed["user"]["balance"].toDouble(), 1234.56, 0.001);
    ASSERT_TRUE(parsed["user"]["balance"].isDouble());
    
    // Verify scores array
    auto scores = parsed["user"]["scores"];
    ASSERT_TRUE(scores.isArray());
    ASSERT_EQ(scores.size(), 4);
    ASSERT_TRUE(scores[0].isInteger());
    ASSERT_TRUE(scores[1].isDouble());
    ASSERT_TRUE(scores[2].isInteger());
    ASSERT_TRUE(scores[3].isDouble());
    
    // Verify metadata
    ASSERT_EQ(parsed["metadata"]["timestamp"].toLongLong(), 1634567890123LL);
    ASSERT_TRUE(parsed["metadata"]["timestamp"].isInteger());
    ASSERT_NEAR(parsed["metadata"]["version"].toDouble(), 2.1, 0.001);
    ASSERT_TRUE(parsed["metadata"]["version"].isDouble());
    
    // Verify ratios array
    auto ratios = parsed["metadata"]["ratios"];
    ASSERT_TRUE(ratios.isArray());
    ASSERT_EQ(ratios.size(), 5);
    for (size_t i = 0; i < ratios.size(); ++i) {
        ASSERT_TRUE(ratios[i].isDouble());
    }
    ASSERT_NEAR(ratios[0].toDouble(), 0.1, 0.001);
    ASSERT_NEAR(ratios[4].toDouble(), 1.0, 0.001);
}

TEST(JsonParsing_StringHandling) {
    // Test string parsing with special characters
    std::string stringJson = R"({
        "simple": "hello world",
        "escaped": "line1\nline2\ttabbed",
        "unicode": "Hello \u4E2D\u6587",
        "empty": "",
        "quotes": "She said \"Hello\""
    })";
    
    auto parsed = JsonValue::parse(stringJson);
    
    ASSERT_TRUE(parsed.isObject());
    ASSERT_EQ(parsed["simple"].toString(), "hello world");
    ASSERT_EQ(parsed["escaped"].toString(), "line1\nline2\ttabbed");
    ASSERT_EQ(parsed["empty"].toString(), "");
    ASSERT_EQ(parsed["quotes"].toString(), "She said \"Hello\"");
    
    // Test that all values are recognized as strings
    ASSERT_TRUE(parsed["simple"].isString());
    ASSERT_TRUE(parsed["escaped"].isString());
    ASSERT_TRUE(parsed["unicode"].isString());
    ASSERT_TRUE(parsed["empty"].isString());
    ASSERT_TRUE(parsed["quotes"].isString());
}

TEST(JsonValue_EscapeString_UnicodeVariants) {
    using JsonStruct::JsonValue;
    ASSERT_EQ(JsonValue::escapeString("A", false), "A");
    ASSERT_EQ(JsonValue::escapeString("中", false), "中");
    ASSERT_EQ(JsonValue::escapeString("€", false), "€");
    ASSERT_EQ(JsonValue::escapeString("😀", false), "😀");
    ASSERT_EQ(JsonValue::escapeString("\x01", false), "\\u0001");
    ASSERT_EQ(JsonValue::escapeString("中", true), "\\u4e2d");
    ASSERT_EQ(JsonValue::escapeString("€", true), "\\u20ac");
    ASSERT_EQ(JsonValue::escapeString("😀", true), "\\ud83d\\ude00");
    ASSERT_EQ(JsonValue::escapeString("A中€😀", true), "A\\u4e2d\\u20ac\\ud83d\\ude00");
    ASSERT_EQ(JsonValue::escapeString("😀😁😂", true), "\\ud83d\\ude00\\ud83d\\ude01\\ud83d\\ude02");
    ASSERT_EQ(JsonValue::escapeString("", false), "");
}

TEST(JsonValue_ParsingBoundaryCondition) {
    using namespace JsonStruct::literals;

	JsonValue::ParseOptions options;
	//options.allowRecovery = true;
    options.allowTrailingCommas = true;
    JsonValue json = JsonValue::parse("[1,2,,,4]", options);
    ASSERT_EQ(json.dump(), "[1,2,4]");

    options.allowRecovery = true;
    JsonValue json2 = JsonValue::parse("[1,2,,,4]", options);
    ASSERT_EQ(json2.dump(), "[1,2,4]");

    JsonValue json3 = JsonValue::parse("[1,2,,3,4]", options);
    ASSERT_EQ(json3.dump(), "[1,2,3,4]");

    JsonValue json4 = JsonValue::parse(R"({"key1": "value1",,,, "key2": "value2",,, "key3": "value3"})", options);
    ASSERT_EQ(json4.dump(), R"({"key1":"value1","key2":"value2","key3":"value3"})");

    options.allowRecovery = false;
    JsonValue json5 = JsonValue::parse(R"({"key1": "value1",, "key2": "value2",, "key3": "value3"})", options);
    ASSERT_EQ(json5.dump(), R"({"key1":"value1","key2":"value2","key3":"value3"})");

    options.allowTrailingCommas = false;
    JsonValue json6;
    std::string errMsg;
    auto err = JsonValue::parse("[1,2,,,4]", json6, errMsg, options);
    ASSERT_NE(err, JsonErrc::Success);

    // recovery mode + allowTrailingCommas = true
    options.allowRecovery = true;
    options.allowTrailingCommas = true;

	// continue multiple illegal keys and commas, without a terminator
    std::string json7 = R"({ ,,, , , , "a":1, , , , "b":2 ,,, })";
    auto value = JsonStruct::JsonValue::parse(json7, options);

    ASSERT_TRUE(value.isObject());
    ASSERT_TRUE(value.contains("a"));
    ASSERT_TRUE(value.contains("b"));
	ASSERT_EQ(value.dump(), R"({"a":1,"b":2})");
}

TEST(JsonValue_ParseUnicodeCondition) {
	std::string vJson = R"("abc\u12def")";
	auto value = JsonValue::parse(vJson);
    std::string json = R"("abc\u12G4def")";
    std::error_code ec;
	std::string errMsg;
    value = JsonValue::parse(json, {}, ec, errMsg);

    // 应该解析失败
    ASSERT_TRUE(value.isNull());
}

int main() {
    std::cout << "=== JSON Parsing Migration Tests ===" << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    if (result == 0) {
        std::cout << "✅ All JSON parsing tests PASSED!" << std::endl;
        std::cout << "🎉 Complete precision support verified!" << std::endl;
    } else {
        std::cout << "❌ Some JSON parsing tests FAILED!" << std::endl;
    }
    
    return result;
}
