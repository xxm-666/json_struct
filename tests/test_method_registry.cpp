#include "json_engine/internal/json_path_filter_evaluator.h"
#include "json_engine/json_value.h"
#include "../test_framework/test_framework.h"

using namespace jsonpath;
using namespace JsonStruct;

// Custom method handler for testing
std::optional<JsonValue> customSumHandler(const JsonStruct::JsonValue& value) {
    if (!value.isArray()) {
        return std::nullopt;
    }

    const auto* arr = value.getArray();
    if (!arr || arr->empty()) {
        return std::nullopt;
    }

    double sum = 0.0;
    for (size_t i = 0; i < arr->size(); ++i) {
        const auto& element = (*arr)[i];
        if (element.isNumber()) {
            auto num_opt = element.getNumber();
            if (num_opt) {
                sum += num_opt.value();
            }
        }
    }

    return JsonValue(sum);
}

// Custom method handler for counting strings
std::optional<JsonValue> customCountStringsHandler(const JsonStruct::JsonValue& value) {
    if (!value.isArray()) {
        return std::nullopt;
    }

    const auto* arr = value.getArray();
    if (!arr || arr->empty()) {
        return JsonValue(0.0);
    }

    double count = 0.0;
    for (size_t i = 0; i < arr->size(); ++i) {
        const auto& element = (*arr)[i];
        if (element.isString()) {
            count += 1.0;
        }
    }

    return JsonValue(count);
}

// 返回字符串类型
std::optional<JsonValue> customHelloHandler(const JsonStruct::JsonValue& value) {
    return JsonValue("hello world");
}

TEST(ReturnStringType) {
    FilterEvaluator::registerMethod("hello", customHelloHandler);
    JsonValue root = JsonValue::parse(R"({})");
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("hello() == 'hello world'", root));
}

// 返回布尔类型
std::optional<JsonValue> customBoolHandler(const JsonStruct::JsonValue& value) {
    return JsonValue(true);
}

TEST(ReturnBoolType) {
    FilterEvaluator::registerMethod("isTrue", customBoolHandler);
    JsonValue root = JsonValue::parse(R"({})");
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("isTrue() == true", root));
    ASSERT_FALSE(FilterEvaluator::evaluateFilterCondition("isTrue() == false", root));
}

// 返回数组类型
//std::optional<JsonValue> customArrayHandler(const JsonStruct::JsonValue& value) {
//    std::vector<JsonValue> arr = { JsonValue(1), JsonValue(2), JsonValue(3) };
//    return JsonValue(arr);
//}
//
//TEST(ReturnArrayType) {
//    FilterEvaluator::registerMethod("array", customArrayHandler);
//    JsonValue root = JsonValue::parse(R"({})");
//    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("array().length() == 3", root));
//}

// 返回对象类型
std::optional<JsonValue> customObjectHandler(const JsonStruct::JsonValue& value) {
    JsonValue obj = JsonValue::parse(R"({ "a": 1, "b": 2 })");
    return obj;
}

TEST(ReturnObjectType) {
    FilterEvaluator::registerMethod("object", customObjectHandler);
    JsonValue root = JsonValue::parse(R"({})");
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("object().a == 1", root));
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("object().b == 2", root));
}

// 返回null类型
std::optional<JsonValue> customNullHandler(const JsonStruct::JsonValue& value) {
    return JsonValue(); // 默认构造为null
}

TEST(ReturnNullType) {
    FilterEvaluator::registerMethod("nullValue", customNullHandler);
    JsonValue root = JsonValue::parse(R"({})");
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("nullValue() == null", root));
}

TEST(BuiltinMethods) {
    JsonValue root = JsonValue::parse(R"({
        "numbers": [1, 2, 3, 4, 5],
        "words": ["hello", "world", "test"],
        "mixed": [1, "hello", 3, "world", 5],
        "name": "test_string"
    })");
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("@.numbers.length() > 3", root));
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("@.numbers.max() == 5", root));
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("@.name.length() == 11", root));
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("@.words.size() == 3", root));
}

TEST(CustomMethods) {
    FilterEvaluator::registerMethod("sum", customSumHandler);
    FilterEvaluator::registerMethod("countStrings", customCountStringsHandler);
    JsonValue root = JsonValue::parse(R"({
        "numbers": [1, 2, 3, 4, 5],
        "words": ["hello", "world", "test"],
        "mixed": [1, "hello", 3, "world", 5]
    })");
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("@.numbers.sum() == 15", root));
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("@.mixed.countStrings() == 2", root));
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("@.numbers.sum() > 10", root));
}

TEST(MethodUnregistration) {
    FilterEvaluator::unregisterMethod("sum");
    JsonValue root = JsonValue::parse(R"({
        "numbers": [1, 2, 3, 4, 5]
    })");
    ASSERT_FALSE(FilterEvaluator::evaluateFilterCondition("@.numbers.sum() == 15", root));
    JsonValue mixed_root = JsonValue::parse(R"({
        "mixed": [1, "hello", 3, "world", 5]
    })");
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("@.mixed.countStrings() == 2", mixed_root));
}

TEST(EdgeCases) {
    JsonValue root = JsonValue::parse(R"({
        "empty_array": [],
        "non_numeric": ["a", "b", "c"],
        "single_item": [42]
    })");
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("@.empty_array.length() == 0", root));
    ASSERT_FALSE(FilterEvaluator::evaluateFilterCondition("@.non_numeric.max() > 0", root));
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("@.single_item.max() == 42", root));
}

TEST(ComplexExpressions) {
    FilterEvaluator::registerMethod("sum", customSumHandler);
    JsonValue root = JsonValue::parse(R"({
        "data": {
            "numbers": [1, 2, 3, 4, 5],
            "values": [10, 20, 30]
        }
    })");
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition(
        "@.data.numbers.length() > 3 && @.data.values.sum() == 60", root));
    FilterEvaluator::registerMethod("alwaysTrue", [](const JsonStruct::JsonValue&) -> std::optional<JsonStruct::JsonValue> {
        return JsonStruct::JsonValue(1.0);
    });
    ASSERT_TRUE(FilterEvaluator::evaluateFilterCondition("alwaysTrue() == 1", root));
}

int main() {
    //FilterEvaluator::clearMethods();
    return RUN_ALL_TESTS();
}
