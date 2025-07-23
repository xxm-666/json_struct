// JSONPath 高级功能测试
#include "../src/jsonstruct.h"
#include "../test_framework/test_framework.h"
#include <iostream>
#include <chrono>

using namespace JsonStruct;

TEST(JsonPath_ComplexExpressions) {
    JsonValue json = JsonValue::parse(R"({
        "store": {
            "book": [
                {
                    "category": "reference",
                    "author": "Nigel Rees",
                    "title": "Sayings of the Century",
                    "price": 8.95,
                    "isbn": "0-553-21311-3"
                },
                {
                    "category": "fiction",
                    "author": "Evelyn Waugh",
                    "title": "Sword of Honour", 
                    "price": 12.99
                },
                {
                    "category": "fiction",
                    "author": "Herman Melville",
                    "title": "Moby Dick",
                    "isbn": "0-553-21311-3",
                    "price": 8.99
                },
                {
                    "category": "fiction",
                    "author": "J. R. R. Tolkien",
                    "title": "The Lord of the Rings",
                    "isbn": "0-395-19395-8",
                    "price": 22.99
                }
            ],
            "bicycle": {
                "color": "red",
                "price": 19.95
            }
        },
        "expensive": 10
    })");

    // 测试多重条件过滤
    auto cheapBooks = json.query("$.store.book[?(@.price < 10)]");
    ASSERT_EQ(cheapBooks.size(), 2); // 两本价格小于10的书
    
    // 测试字符串比较
    auto fictionBooks = json.query("$.store.book[?(@.category == 'fiction')]");
    ASSERT_EQ(fictionBooks.size(), 3); // 三本小说
    
    // 测试字段存在性检查
    auto booksWithIsbn = json.query("$.store.book[?(@.isbn)]");
    ASSERT_EQ(booksWithIsbn.size(), 3); // 三本有ISBN的书
}

TEST(JsonPath_AdvancedArrayOperations) {
    JsonValue json = JsonValue::parse(R"({
        "matrix": [
            [1, 2, 3],
            [4, 5, 6], 
            [7, 8, 9]
        ],
        "nested": {
            "arrays": [
                {"data": [10, 20, 30]},
                {"data": [40, 50, 60]},
                {"data": [70, 80, 90]}
            ]
        }
    })");

    // 测试多维数组访问
    auto element = json.query("$.matrix[1][2]");
    ASSERT_EQ(element.size(), 1);
    ASSERT_EQ(element[0].toInt(), 6);
    
    // 测试嵌套数组的通配符
    auto allData = json.query("$.nested.arrays[*].data[*]");
    ASSERT_EQ(allData.size(), 9); // 3个数组 × 3个元素
    
    // 测试数组长度函数（如果支持）
    auto firstRow = json.query("$.matrix[0]");
    ASSERT_EQ(firstRow.size(), 1);
    ASSERT_TRUE(firstRow[0].isArray());
    ASSERT_EQ(firstRow[0].toArray().size(), 3);
}

TEST(JsonPath_RecursiveDescentAdvanced) {
    JsonValue json = JsonValue::parse(R"({
        "company": {
            "departments": {
                "engineering": {
                    "teams": {
                        "backend": {
                            "members": [
                                {"name": "Alice", "role": "senior"},
                                {"name": "Bob", "role": "junior"}
                            ]
                        },
                        "frontend": {
                            "members": [
                                {"name": "Charlie", "role": "senior"},
                                {"name": "David", "role": "junior"}
                            ]
                        }
                    }
                },
                "sales": {
                    "teams": {
                        "enterprise": {
                            "members": [
                                {"name": "Eve", "role": "manager"}
                            ]
                        }
                    }
                }
            }
        }
    })");

    // 测试深度递归查询
    auto allMembers = json.query("$..members");
    ASSERT_EQ(allMembers.size(), 3); // 3个teams的members数组
    
    // 测试递归查询特定属性
    auto allNames = json.query("$..name");
    ASSERT_EQ(allNames.size(), 5); // 5个成员的名字
    
    // 测试递归查询with条件
    auto seniorMembers = json.query("$..members[?(@.role == 'senior')]");
    ASSERT_EQ(seniorMembers.size(), 2); // Alice 和 Charlie
}

TEST(JsonPath_UnionExpressions) {
    JsonValue json = JsonValue::parse(R"({
        "data": {
            "primary": {"value": 100},
            "secondary": {"value": 200}, 
            "tertiary": {"value": 300},
            "items": [1, 2, 3, 4, 5]
        }
    })");

    // 测试联合查询（多个路径）
    auto unionResult = json.query("$.data.primary.value,$.data.secondary.value");
    ASSERT_EQ(unionResult.size(), 2);
    ASSERT_EQ(unionResult[0].toInt(), 100);
    ASSERT_EQ(unionResult[1].toInt(), 200);
    
    // 测试数组索引联合
    auto multiIndex = json.query("$.data.items[0,2,4]");
    ASSERT_EQ(multiIndex.size(), 3);
    ASSERT_EQ(multiIndex[0].toInt(), 1);
    ASSERT_EQ(multiIndex[1].toInt(), 3);
    ASSERT_EQ(multiIndex[2].toInt(), 5);
}

TEST(JsonPath_SlicingExtended) {
    JsonValue json = JsonValue::parse(R"({
        "sequence": [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]
    })");

    // 测试步长切片 [start:end:step]
    auto stepSlice = json.query("$.sequence[0:10:2]");
    ASSERT_EQ(stepSlice.size(), 5); // 0, 2, 4, 6, 8
    
    // 测试负步长（反向）
    auto reverseSlice = json.query("$.sequence[10:0:-2]");
    ASSERT_EQ(reverseSlice.size(), 5); // 10, 8, 6, 4, 2
    
    // 测试复杂切片
    auto complexSlice = json.query("$.sequence[-5:-1]");
    ASSERT_EQ(complexSlice.size(), 4); // 最后5个元素中的前4个
}

TEST(JsonPath_FilterExpressions) {
    JsonValue json = JsonValue::parse(R"({
        "products": [
            {"id": 1, "name": "Widget A", "price": 10.50, "tags": ["electronics", "gadget"]},
            {"id": 2, "name": "Widget B", "price": 25.00, "tags": ["electronics", "tool"]},
            {"id": 3, "name": "Book C", "price": 15.99, "tags": ["education", "book"]},
            {"id": 4, "name": "Gadget D", "price": 8.99, "tags": ["electronics", "gadget", "cheap"]}
        ]
    })");

    // 测试复杂比较表达式
    auto midRangeProducts = json.query("$.products[?(@.price > 10 && @.price < 20)]");
    ASSERT_EQ(midRangeProducts.size(), 2); // Widget A 和 Book C
    
    // 测试字符串包含检查
    auto widgetProducts = json.query("$.products[?(@.name =~ /Widget.*/)]");
    ASSERT_EQ(widgetProducts.size(), 2); // Widget A 和 Widget B
    
    // 测试数组包含检查
    auto gadgetProducts = json.query("$.products[?('gadget' in @.tags)]");
    ASSERT_EQ(gadgetProducts.size(), 2); // Widget A 和 Gadget D
}

TEST(JsonPath_FunctionExpressions) {
    JsonValue json = JsonValue::parse(R"({
        "arrays": [
            {"numbers": [1, 2, 3, 4, 5]},
            {"numbers": [10, 20, 30]}, 
            {"numbers": [100]}
        ],
        "strings": [
            {"text": "hello world"},
            {"text": "foo bar"},
            {"text": "test"}
        ]
    })");

    // 测试长度函数
    auto longArrays = json.query("$.arrays[?(@.numbers.length() > 3)]");
    ASSERT_EQ(longArrays.size(), 1); // 只有第一个数组长度>3
    
    // 测试字符串函数
    auto longStrings = json.query("$.strings[?(@.text.length() > 5)]");
    ASSERT_EQ(longStrings.size(), 2); // "hello world" 和 "foo bar"
    
    // 测试最大值/最小值函数
    auto hasLargeNumbers = json.query("$.arrays[?(@.numbers.max() > 50)]");
    ASSERT_EQ(hasLargeNumbers.size(), 1); // 只有第三个数组有100
}

TEST(JsonPath_EdgeCases) {
    JsonValue json = JsonValue::parse(R"({
        "empty": {},
        "nullValue": null,
        "emptyArray": [],
        "emptyString": "",
        "zero": 0,
        "false": false,
        "special": {
            "key with spaces": "value1",
            "key-with-dashes": "value2", 
            "key.with.dots": "value3"
        }
    })");

    // 测试空值处理
    auto nullAccess = json.query("$.nullValue");
    ASSERT_EQ(nullAccess.size(), 1);
    ASSERT_TRUE(nullAccess[0].isNull());
    
    // 测试空数组/对象
    auto emptyObjAccess = json.query("$.empty");
    ASSERT_EQ(emptyObjAccess.size(), 1);
    ASSERT_TRUE(emptyObjAccess[0].isObject());
    
    // 测试特殊键名
    auto specialKeys = json.query("$.special['key with spaces']");
    ASSERT_EQ(specialKeys.size(), 1);
    ASSERT_EQ(specialKeys[0].toString(), "value1");
    
    // 测试点号键名
    auto dotKeys = json.query("$.special['key.with.dots']");
    ASSERT_EQ(dotKeys.size(), 1);
    ASSERT_EQ(dotKeys[0].toString(), "value3");
}

TEST(JsonPath_PerformanceOptimization) {
    // 创建大型JSON结构
    JsonValue::ObjectType largeObj;
    for (int i = 0; i < 1000; ++i) {
        JsonValue::ObjectType item;
        item["id"] = JsonValue(i);
        item["name"] = JsonValue("Item " + std::to_string(i));
        item["value"] = JsonValue(i * 1.5);
        largeObj["item_" + std::to_string(i)] = JsonValue(item);
    }
    JsonValue json(largeObj);

    // 测试大数据集上的查询性能
    auto start = std::chrono::high_resolution_clock::now();
    auto results = json.query("$..value");
    auto end = std::chrono::high_resolution_clock::now();
    
    ASSERT_EQ(results.size(), 1000);
    
    // 确保查询在合理时间内完成（1秒）
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    ASSERT_TRUE(duration.count() < 1000);
}