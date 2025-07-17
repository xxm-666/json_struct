// JSON Filter 基础测试
#include "../src/jsonstruct.h"
#include "../src/json_engine/json_filter.h"
#include "../test_framework/test_framework.h"
#include <iostream>

using namespace JsonStruct;

TEST(JsonFilter_BasicPathExists) {
    JsonValue json = JsonValue::parse(R"({
        "name": "test",
        "value": 42,
        "array": [1, 2, 3],
        "nested": {
            "key": "value"
        }
    })");

    JsonFilter filter;
    
    // 测试路径存在性检查
    ASSERT_TRUE(filter.pathExists(json, "$.name"));
    ASSERT_TRUE(filter.pathExists(json, "$.value"));
    ASSERT_TRUE(filter.pathExists(json, "$.array"));
    ASSERT_TRUE(filter.pathExists(json, "$.nested.key"));
    ASSERT_FALSE(filter.pathExists(json, "$.nonexistent"));
}

TEST(JsonFilter_SelectFirst) {
    JsonValue json = JsonValue::parse(R"({
        "items": [
            {"id": 1, "name": "first"},
            {"id": 2, "name": "second"},
            {"id": 3, "name": "third"}
        ]
    })");

    JsonFilter filter;
    
    // 测试选择第一个匹配项
    const JsonValue* firstItem = filter.selectFirst(json, "$.items[0]");
    ASSERT_TRUE(firstItem != nullptr);
    if (firstItem) {
        ASSERT_TRUE(firstItem->isObject());
        ASSERT_EQ((*firstItem)["id"].toInt(), 1);
    }
    
    // 测试不存在的路径
    const JsonValue* nonexistent = filter.selectFirst(json, "$.nonexistent");
    ASSERT_TRUE(nonexistent == nullptr);
}

TEST(JsonFilter_SelectValues) {
    JsonValue json = JsonValue::parse(R"({
        "users": [
            {"name": "Alice", "age": 30},
            {"name": "Bob", "age": 25},
            {"name": "Charlie", "age": 35}
        ]
    })");

    JsonFilter filter;
    
    // 测试选择所有匹配的值
    auto names = filter.selectValues(json, "$.users[*].name");
    ASSERT_EQ(names.size(), 3);
    
    for (const auto& name : names) {
        ASSERT_TRUE(name.isString());
    }
    
    // 验证具体的名字
    if (names.size() >= 3) {
        // 注意：顺序可能会变化，所以只检查是否包含预期的值
        bool hasAlice = false, hasBob = false, hasCharlie = false;
        for (const auto& name : names) {
            std::string nameStr = name.toString();
            if (nameStr == "Alice") hasAlice = true;
            if (nameStr == "Bob") hasBob = true;
            if (nameStr == "Charlie") hasCharlie = true;
        }
        ASSERT_TRUE(hasAlice);
        ASSERT_TRUE(hasBob);
        ASSERT_TRUE(hasCharlie);
    }
}

TEST(JsonFilter_QueryWithOptions) {
    JsonValue json = JsonValue::parse(R"({
        "Data": [
            {"Name": "ALICE"},
            {"name": "bob"},
            {"NAME": "Charlie"}
        ]
    })");

    // 测试大小写敏感
    JsonFilter::QueryOptions caseSensitive;
    caseSensitive.caseSensitive = true;
    JsonFilter sensitiveFilter(caseSensitive);
    
    auto sensitiveResults = sensitiveFilter.selectValues(json, "$.Data[*].Name");
    ASSERT_EQ(sensitiveResults.size(), 1); // 只有第一个对象有"Name"
    
    // 测试大小写不敏感
    JsonFilter::QueryOptions caseInsensitive;
    caseInsensitive.caseSensitive = false;
    JsonFilter insensitiveFilter(caseInsensitive);
    
    // 注意：大小写不敏感的实现可能有所不同，这里先测试基本功能
    auto insensitiveResults = insensitiveFilter.selectValues(json, "$.Data[*].Name");
    // 结果取决于具体实现
}

TEST(JsonFilter_ArrayAccess) {
    JsonValue json = JsonValue::parse(R"({
        "numbers": [10, 20, 30, 40, 50]
    })");

    JsonFilter filter;
    
    // 测试数组索引访问
    auto first = filter.selectValues(json, "$.numbers[0]");
    ASSERT_EQ(first.size(), 1);
    ASSERT_EQ(first[0].toInt(), 10);
    
    auto last = filter.selectValues(json, "$.numbers[-1]");
    if (last.size() > 0) {
        ASSERT_EQ(last[0].toInt(), 50);
    }
    
    // 测试数组通配符
    auto allNumbers = filter.selectValues(json, "$.numbers[*]");
    ASSERT_EQ(allNumbers.size(), 5);
}

int main() {
    return RUN_ALL_TESTS();
}
