#include <iostream>
#include <string>
#include "json_value.h"
#include "json_filter.h"

using namespace JsonStruct;
using namespace JsonStruct::filter_types;

int main() {
    // 创建测试 JSON 数据
    JsonValue testData = JsonValue::parse(R"({
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
        }
    })");

    std::cout << "=== JsonFilter 使用示例 ===" << std::endl;

    // ===== 基础用法 =====
    std::cout << "\n--- 基础查询方法 ---" << std::endl;
    
    // 创建过滤器实例
    JsonFilter filter = JsonFilter::createDefault();
    
    // 1. 路径存在检查
    bool exists = filter.pathExists(testData, "$.store.book[0].title");
    std::cout << "路径 '$.store.book[0].title' 存在: " << (exists ? "是" : "否") << std::endl;

    // 2. 选择第一个匹配项
    const JsonValue* firstBook = filter.selectFirst(testData, "$.store.book[0]");
    if (firstBook) {
        std::cout << "第一本书: " << firstBook->dump(2) << std::endl;
    }

    // 3. 选择所有匹配项
    auto allBooks = filter.selectAll(testData, "$.store.book[*]");
    std::cout << "找到 " << allBooks.size() << " 本书" << std::endl;

    // 4. 获取值的副本
    auto bookTitles = filter.selectValues(testData, "$.store.book[*].title");
    std::cout << "书籍标题数量: " << bookTitles.size() << std::endl;

    // ===== 高级用法 =====
    std::cout << "\n--- 高级查询方法 ---" << std::endl;

    // 1. 详细查询
    auto queryResults = filter.query(testData, "$.store.book[*].price");
    std::cout << "价格查询结果:" << std::endl;
    for (const auto& result : queryResults) {
        std::cout << "  路径: " << result.path << ", 值: " << result.value->dump() << std::endl;
    }

    // 2. 自定义过滤器
    auto expensiveBooks = filter.queryWithFilter(testData, 
        JsonFilter::Filters::byNumberRange(10.0, 100.0)
    );
    std::cout << "价格在 10-100 之间的项目数量: " << expensiveBooks.size() << std::endl;

    // 3. 正则表达式过滤
    auto priceResults = filter.queryWithRegex(testData, R"(.*\.price$)");
    std::cout << "所有价格字段数量: " << priceResults.size() << std::endl;

    // ===== 链式查询 =====
    std::cout << "\n--- 链式查询 API ---" << std::endl;

    // 查找所有小说类书籍并按价格排序
    auto fictionBooks = filter.from(testData)
        .where("$.store.book[*]")
        .where(JsonFilter::Filters::hasProperty("category"))
        .limit(2)
        .execute();
    
    std::cout << "找到的书籍数量 (限制2本): " << fictionBooks.size() << std::endl;

    // 统计查询
    size_t bookCount = filter.from(testData)
        .where("$.store.book[*]")
        .count();
    std::cout << "书籍总数: " << bookCount << std::endl;

    bool hasExpensiveBooks = filter.from(testData)
        .where("$.store.book[*].price")
        .any();
    std::cout << "有价格信息的书籍: " << (hasExpensiveBooks ? "是" : "否") << std::endl;

    // ===== 批量操作 =====
    std::cout << "\n--- 批量操作 ---" << std::endl;

    // 批量查询
    std::vector<std::string> queries = {
        "$.store.book[*].title",
        "$.store.book[*].author",
        "$.store.book[*].price"
    };

    auto batchResults = filter.batchQuery(testData, queries);
    std::cout << "批量查询结果:" << std::endl;
    for (size_t i = 0; i < queries.size(); ++i) {
        std::cout << "  查询 '" << queries[i] << "': " << batchResults[i].size() << " 项结果" << std::endl;
    }

    // 转换操作
    auto titles = filter.transform(queryResults, 
        [](const JsonValue& value, const std::string& path) -> JsonValue {
            return JsonValue("价格: " + value.toString());
        }
    );
    std::cout << "转换后的结果数量: " << titles.size() << std::endl;

    // ===== 不同配置的过滤器 =====
    std::cout << "\n--- 不同配置的过滤器 ---" << std::endl;

    // 高性能过滤器（启用缓存）
    JsonFilter highPerfFilter = JsonFilter::createHighPerformance();
    auto cachedResult1 = highPerfFilter.selectAll(testData, "$.store.book[*].title");
    auto cachedResult2 = highPerfFilter.selectAll(testData, "$.store.book[*].title"); // 应该从缓存获取
    std::cout << "高性能过滤器结果: " << cachedResult1.size() << " 项" << std::endl;

    // 严格模式过滤器（禁用通配符等高级功能）
    JsonFilter strictFilter = JsonFilter::createStrict();
    auto strictResult = strictFilter.selectFirst(testData, "$.store.book[0].title");
    std::cout << "严格模式查询结果: " << (strictResult ? "成功" : "失败") << std::endl;

    // ===== 预定义过滤器示例 =====
    std::cout << "\n--- 预定义过滤器 ---" << std::endl;

    // 按类型过滤
    auto stringValues = filter.queryWithFilter(testData, JsonFilter::Filters::byType(String));
    std::cout << "字符串类型值数量: " << stringValues.size() << std::endl;

    // 按字符串值过滤
    auto fictionFilter = filter.queryWithFilter(testData, JsonFilter::Filters::byString("fiction", true));
    std::cout << "值为 'fiction' 的项目数量: " << fictionFilter.size() << std::endl;

    // 非空过滤器
    auto nonEmptyValues = filter.queryWithFilter(testData, JsonFilter::Filters::isNotEmpty());
    std::cout << "非空值数量: " << nonEmptyValues.size() << std::endl;

    // ===== 便利函数 =====
    std::cout << "\n--- 便利函数 ---" << std::endl;

    // 使用全局便利函数
    bool quickExists = query::exists(testData, "$.store.bicycle.color");
    std::cout << "快速存在检查: " << (quickExists ? "是" : "否") << std::endl;

    auto quickFirst = query::first(testData, "$.store.bicycle.price");
    if (quickFirst) {
        std::cout << "自行车价格: " << quickFirst->dump() << std::endl;
    }

    auto quickAll = query::all(testData, "$.store.book[*].author");
    std::cout << "所有作者数量: " << quickAll.size() << std::endl;

    // 链式便利函数
    auto quickChain = query::from(testData)
        .where("$.store.book[*]")
        .where(JsonFilter::Filters::hasProperty("isbn"))
        .values();
    std::cout << "有 ISBN 的书籍数量: " << quickChain.size() << std::endl;

    std::cout << "\n=== 示例完成 ===" << std::endl;

    return 0;
}
