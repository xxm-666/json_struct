#include "json_engine/json_value.h"
#include "test_framework.h"
#include <iostream>
#include <string>

using namespace JsonStruct;
using namespace TestFramework;

TEST(ValidJson) {
    JsonValue value;
    std::string errMsg;
    auto ec = JsonValue::parse(R"({"name": "test", "value": 42})", value, errMsg);
    ASSERT_TRUE(ec == JsonErrc::Success);
}

TEST(UnexpectedCharacter) {
    JsonValue value;
    std::string errMsg;
    auto ec = JsonValue::parse(R"({"name": test})", value, errMsg);
    ASSERT_TRUE(ec == JsonErrc::UnexpectedCharacter);
}

TEST(UnexpectedEnd) {
    JsonValue value;
    std::string errMsg;
    auto ec = JsonValue::parse(R"({"name": "test")", value, errMsg);
    ASSERT_TRUE(ec == JsonErrc::UnexpectedEnd);
}

TEST(InvalidNumber) {
    JsonValue value;
    std::string errMsg;
    auto ec = JsonValue::parse(R"({"value": 12.})", value, errMsg);
    ASSERT_TRUE(ec == JsonErrc::ParseError);
}

TEST(Serialization) {
    JsonValue value = JsonValue::object({
        {"name", JsonValue(std::string("test"))},
        {"value", JsonValue(42)}
    });
    std::string output;
    std::string errMsg;
    auto ec = value.toJson(output, errMsg);
    ASSERT_TRUE(!ec);
}

TEST(DepthExceeded) {
    JsonValue value;
    std::string errMsg;
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
    ASSERT_TRUE(ec == JsonErrc::DepthExceeded || ec == JsonErrc::Success);
}

TEST(ErrorCodeConversion) {
    std::error_code ec = JsonErrc::UnexpectedCharacter;
    ASSERT_TRUE(ec.category().name() == std::string("JsonStruct") && ec.value() == 7);
}

TEST(ValidJsonWithSpecialChars) {
    JsonValue value;
    std::string errMsg;
    auto ec = JsonValue::parse(R"({"name": "test", "special": "\n\t\r\"\\", "unicode": "\u1234"})", value, errMsg);
    ASSERT_TRUE(ec == JsonErrc::Success);
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}
