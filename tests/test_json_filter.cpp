// JSON Filter 功能全面测试
#include "../src/jsonstruct.h"
#include "../src/json_engine/json_filter.h"
#include "../test_framework/test_framework.h"
#include <iostream>
#include <algorithm>

using namespace JsonStruct;

TEST(JsonFilter_BasicFiltering) {
    // 创建测试JSON数据
    JsonValue json = JsonValue::parse(R"({
        "store": {
            "book": [
                {
                    "category": "reference",
                    "author": "Nigel Rees",
                    "title": "Sayings of the Century",
                    "price": 8.95
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
                    "price": 8.99
                }
            ],
            "bicycle": {
                "color": "red",
                "price": 19.95
            }
        }
    })");

    JsonFilter filter;
    
    // 测试基础路径查询
    auto books = filter.selectValues(json, "$.store.book");
    ASSERT_TRUE(books.size() > 0);
    if (books.size() > 0) {
        ASSERT_TRUE(books[0].isArray());
    }
    
    // 测试数组元素访问
    auto firstBook = filter.selectValues(json, "$.store.book[0]");
    ASSERT_TRUE(firstBook.size() > 0);
    if (firstBook.size() > 0) {
        ASSERT_TRUE(firstBook[0].isObject());
    }
    
    // 测试属性访问
    auto authors = filter.selectValues(json, "$.store.book[*].author");
    ASSERT_EQ(authors.size(), 3);
    for (const auto& author : authors) {
        ASSERT_TRUE(author.isString());
    }
}

TEST(JsonFilter_WildcardQueries) {
    JsonValue json = JsonValue::parse(R"({
        "data": {
            "users": [
                {"name": "Alice", "age": 30},
                {"name": "Bob", "age": 25},
                {"name": "Charlie", "age": 35}
            ],
            "products": [
                {"name": "Laptop", "price": 999.99},
                {"name": "Phone", "price": 599.99}
            ]
        }
    })");

    JsonFilter filter;
    
    // 测试通配符查询
    auto allNames = filter.selectValues(json, "$.data.*.*.name");
    ASSERT_EQ(allNames.size(), 5); // 3 users + 2 products
    
    // 测试数组通配符
    auto userAges = filter.selectValues(json, "$.data.users[*].age");
    ASSERT_EQ(userAges.size(), 3);
    
    for (const auto& age : userAges) {
        ASSERT_TRUE(age.isNumber());
        if (age.getInteger().has_value()) {
            ASSERT_TRUE(age.getInteger().value() >= 25);
        }
    }
}

TEST(JsonFilter_RecursiveDescent) {
    JsonValue json = JsonValue::parse(R"({
        "level1": {
            "price": 10.0,
            "level2": {
                "price": 20.0,
                "level3": {
                    "price": 30.0,
                    "items": [
                        {"price": 5.0},
                        {"price": 15.0}
                    ]
                }
            }
        },
        "price": 100.0
    })");

    JsonFilter filter(json);
    
    // 测试递归查询 - 找到所有价格
    auto allPrices = filter.query("$..price");
    ASSERT_EQ(allPrices.size(), 5); // 4个对象级别的价格 + 2个数组中的价格
    
    // 验证价格值
    std::vector<double> expectedPrices = {100.0, 10.0, 20.0, 30.0, 5.0, 15.0};
    std::vector<double> actualPrices;
    for (const auto& price : allPrices) {
        ASSERT_TRUE(price.isNumber());
        actualPrices.push_back(price.toDouble());
    }
    
    // 排序后比较（因为递归查询可能返回不同顺序）
    std::sort(expectedPrices.begin(), expectedPrices.end());
    std::sort(actualPrices.begin(), actualPrices.end());
    ASSERT_EQ(actualPrices, expectedPrices);
}

TEST(JsonFilter_ArraySlicing) {
    JsonValue json = JsonValue::parse(R"({
        "numbers": [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    })");

    JsonFilter filter(json);
    
    // 测试数组切片 [2:5]
    auto slice1 = filter.query("$.numbers[2:5]");
    ASSERT_EQ(slice1.size(), 3); // 元素2, 3, 4
    
    // 测试从开始切片 [:3]
    auto slice2 = filter.query("$.numbers[:3]");
    ASSERT_EQ(slice2.size(), 3); // 元素0, 1, 2
    
    // 测试到结尾切片 [7:]
    auto slice3 = filter.query("$.numbers[7:]");
    ASSERT_EQ(slice3.size(), 3); // 元素7, 8, 9
    
    // 测试负索引 [-3:]
    auto slice4 = filter.query("$.numbers[-3:]");
    ASSERT_EQ(slice4.size(), 3); // 最后3个元素
}

TEST(JsonFilter_ConditionalFilters) {
    JsonValue json = JsonValue::parse(R"({
        "products": [
            {"name": "Cheap Item", "price": 5.0, "inStock": true},
            {"name": "Medium Item", "price": 15.0, "inStock": false},
            {"name": "Expensive Item", "price": 25.0, "inStock": true},
            {"name": "Budget Item", "price": 8.0, "inStock": true}
        ]
    })");

    JsonFilter filter(json);
    
    // 测试价格过滤（小于10）
    auto cheapItems = filter.query("$.products[?(@.price < 10)]");
    ASSERT_EQ(cheapItems.size(), 2); // Cheap Item 和 Budget Item
    
    // 测试库存过滤
    auto inStockItems = filter.query("$.products[?(@.inStock == true)]");
    ASSERT_EQ(inStockItems.size(), 3);
    
    // 测试复合条件
    auto cheapInStock = filter.query("$.products[?(@.price < 10 && @.inStock == true)]");
    ASSERT_EQ(cheapInStock.size(), 2);
}

TEST(JsonFilter_QueryOptions) {
    JsonValue json = JsonValue::parse(R"({
        "Data": [
            {"Name": "ALICE", "data": {"value": 1}},
            {"name": "bob", "data": {"value": 2}},
            {"NAME": "Charlie", "data": {"value": 3}}
        ]
    })");

    JsonFilter filter(json);
    
    // 测试大小写敏感查询
    JsonFilter::QueryOptions caseSensitive;
    caseSensitive.caseSensitive = true;
    auto sensitiveResult = filter.query("$.Data[*].Name", caseSensitive);
    ASSERT_EQ(sensitiveResult.size(), 1); // 只有第一个对象有"Name"
    
    // 测试大小写不敏感查询
    JsonFilter::QueryOptions caseInsensitive;
    caseInsensitive.caseSensitive = false;
    auto insensitiveResult = filter.query("$.Data[*].name", caseInsensitive);
    ASSERT_EQ(insensitiveResult.size(), 3); // 所有三个对象都匹配
    
    // 测试最大结果限制
    JsonFilter::QueryOptions limitResults;
    limitResults.maxResults = 2;
    auto limitedResult = filter.query("$.Data[*].data.value", limitResults);
    ASSERT_EQ(limitedResult.size(), 2);
}

TEST(JsonFilter_ChainedQueries) {
    JsonValue json = JsonValue::parse(R"({
        "departments": [
            {
                "name": "Sales",
                "employees": [
                    {"name": "John", "salary": 50000},
                    {"name": "Jane", "salary": 55000}
                ]
            },
            {
                "name": "Engineering", 
                "employees": [
                    {"name": "Bob", "salary": 70000},
                    {"name": "Alice", "salary": 75000}
                ]
            }
        ]
    })");

    JsonFilter filter(json);
    
    // 链式查询：先找到所有员工，再过滤高薪员工
    auto allEmployees = filter.query("$.departments[*].employees[*]");
    JsonFilter employeeFilter(allEmployees[0]); // 假设第一个结果是数组
    
    // 测试链式过滤
    auto highSalaryEmployees = filter.query("$.departments[*].employees[?(@.salary > 60000)]");
    ASSERT_EQ(highSalaryEmployees.size(), 2); // Bob 和 Alice
}

TEST(JsonFilter_ErrorHandling) {
    JsonValue json = JsonValue::parse(R"({"valid": "data"})");
    JsonFilter filter(json);
    
    // 测试无效路径
    auto invalidResult = filter.query("$.nonexistent.path");
    ASSERT_TRUE(invalidResult.empty());
    
    // 测试无效语法
    bool exceptionCaught = false;
    try {
        auto result = filter.query("$.[invalid syntax");
    } catch (const std::exception&) {
        exceptionCaught = true;
    }
    ASSERT_TRUE(exceptionCaught);
    
    // 测试数组越界
    auto outOfBounds = filter.query("$.valid[999]");
    ASSERT_TRUE(outOfBounds.empty());
}
