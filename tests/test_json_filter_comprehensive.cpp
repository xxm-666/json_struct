#include <iostream>
#include <string>
#include <cassert>
#include "json_value.h"
#include "json_filter.h"

using namespace JsonStruct;
using namespace JsonStruct::filter_types;

void testBasicPaths(const JsonValue& testData, JsonFilter& filter) {
    std::cout << "\n=== Basic Path Tests ===" << std::endl;
    
    // Test root path
    assert(filter.pathExists(testData, "$"));
    std::cout << "✓ Root path exists" << std::endl;
    
    // Test simple property access
    assert(filter.pathExists(testData, "$.store"));
    assert(filter.pathExists(testData, "$.store.bicycle"));
    assert(filter.pathExists(testData, "$.store.bicycle.color"));
    std::cout << "✓ Simple property access" << std::endl;
    
    // Test array access
    assert(filter.pathExists(testData, "$.store.book"));
    assert(filter.pathExists(testData, "$.store.book[0]"));
    assert(filter.pathExists(testData, "$.store.book[3]"));
    assert(!filter.pathExists(testData, "$.store.book[10]")); // Out of bounds
    std::cout << "✓ Array index access" << std::endl;
    
    // Test nested property access
    assert(filter.pathExists(testData, "$.store.book[0].title"));
    assert(filter.pathExists(testData, "$.store.book[2].isbn"));
    assert(!filter.pathExists(testData, "$.store.book[0].isbn")); // First book has no ISBN
    std::cout << "✓ Nested property access" << std::endl;
}

void testWildcardPaths(const JsonValue& testData, JsonFilter& filter) {
    std::cout << "\n=== Wildcard Path Tests ===" << std::endl;
    
    // Test array wildcard
    auto allBooks = filter.selectAll(testData, "$.store.book[*]");
    assert(allBooks.size() == 4);
    std::cout << "✓ Array wildcard [*]: " << allBooks.size() << " books" << std::endl;
    
    // Test property wildcard on object
    auto storeProperties = filter.selectAll(testData, "$.store.*");
    assert(storeProperties.size() == 2); // book and bicycle
    std::cout << "✓ Object wildcard *: " << storeProperties.size() << " properties" << std::endl;
    
    // Test nested wildcard
    auto allTitles = filter.selectAll(testData, "$.store.book[*].title");
    assert(allTitles.size() == 4);
    std::cout << "✓ Nested wildcard: " << allTitles.size() << " titles" << std::endl;
    
    // Test nested wildcard for prices
    auto allPrices = filter.selectAll(testData, "$.store.book[*].price");
    assert(allPrices.size() == 4);
    std::cout << "✓ Nested wildcard prices: " << allPrices.size() << " prices" << std::endl;
    
    // Test nested wildcard for optional properties
    auto allIsbns = filter.selectAll(testData, "$.store.book[*].isbn");
    assert(allIsbns.size() == 2); // Only 2 books have ISBN
    std::cout << "✓ Optional property wildcard: " << allIsbns.size() << " ISBNs" << std::endl;
}

void testComplexQueries(const JsonValue& testData, JsonFilter& filter) {
    std::cout << "\n=== Complex Query Tests ===" << std::endl;
    
    // Test detailed query results
    auto detailResults = filter.query(testData, "$.store.book[*].author");
    assert(detailResults.size() == 4);
    
    for (const auto& result : detailResults) {
        assert(result.value != nullptr);
        assert(!result.path.empty());
        assert(result.value->isString());
        std::cout << "  Author at " << result.path << ": " << result.value->toString() << std::endl;
    }
    std::cout << "✓ Detailed query results with paths" << std::endl;
    
    // Test value copies
    auto authorValues = filter.selectValues(testData, "$.store.book[*].author");
    assert(authorValues.size() == 4);
    std::cout << "✓ Value copies: " << authorValues.size() << " authors" << std::endl;
}

void testCustomFilters(const JsonValue& testData, JsonFilter& filter) {
    std::cout << "\n=== Custom Filter Tests ===" << std::endl;
    
    // Test type filters
    auto stringValues = filter.queryWithFilter(testData, JsonFilter::Filters::byType(String));
    std::cout << "✓ String type filter: " << stringValues.size() << " strings" << std::endl;
    
    auto numberValues = filter.queryWithFilter(testData, JsonFilter::Filters::byType(Number));
    std::cout << "✓ Number type filter: " << numberValues.size() << " numbers" << std::endl;
    
    auto objectValues = filter.queryWithFilter(testData, JsonFilter::Filters::byType(Object));
    std::cout << "✓ Object type filter: " << objectValues.size() << " objects" << std::endl;
    
    // Test value filters
    auto fictionBooks = filter.queryWithFilter(testData, JsonFilter::Filters::byString("fiction"));
    assert(fictionBooks.size() == 3); // 3 fiction books
    std::cout << "✓ String value filter: " << fictionBooks.size() << " 'fiction' values" << std::endl;
    
    // Test number range filter
    auto expensiveItems = filter.queryWithFilter(testData, JsonFilter::Filters::byNumberRange(10.0, 25.0));
    std::cout << "✓ Number range filter: " << expensiveItems.size() << " items priced 10-25" << std::endl;
    
    // Test property existence filter
    auto itemsWithISBN = filter.queryWithFilter(testData, JsonFilter::Filters::hasProperty("isbn"));
    assert(itemsWithISBN.size() == 2); // Only 2 books have ISBN
    std::cout << "✓ Property existence filter: " << itemsWithISBN.size() << " items with ISBN" << std::endl;
    
    // Test empty/non-empty filters
    auto nonEmptyItems = filter.queryWithFilter(testData, JsonFilter::Filters::isNotEmpty());
    std::cout << "✓ Non-empty filter: " << nonEmptyItems.size() << " non-empty items" << std::endl;
}

void testChainQueries(const JsonValue& testData, JsonFilter& filter) {
    std::cout << "\n=== Chain Query Tests ===" << std::endl;
    
    // Test basic chain
    auto books = filter.from(testData)
        .where("$.store.book[*]")
        .execute();
    assert(books.size() == 4);
    std::cout << "✓ Basic chain query: " << books.size() << " books" << std::endl;
    
    // Test chain with limit
    auto limitedBooks = filter.from(testData)
        .where("$.store.book[*]")
        .limit(2)
        .execute();
    assert(limitedBooks.size() == 2);
    std::cout << "✓ Chain with limit: " << limitedBooks.size() << " books" << std::endl;
    
    // Test chain with custom filter
    auto booksWithISBN = filter.from(testData)
        .where("$.store.book[*]")
        .where(JsonFilter::Filters::hasProperty("isbn"))
        .execute();
    assert(booksWithISBN.size() == 2);
    std::cout << "✓ Chain with custom filter: " << booksWithISBN.size() << " books with ISBN" << std::endl;
    
    // Test statistical operations
    size_t bookCount = filter.from(testData).where("$.store.book[*]").count();
    assert(bookCount == 4);
    std::cout << "✓ Count operation: " << bookCount << " books" << std::endl;
    
    bool hasBooks = filter.from(testData).where("$.store.book[*]").any();
    assert(hasBooks);
    std::cout << "✓ Any operation: " << (hasBooks ? "has books" : "no books") << std::endl;
    
    // Test value extraction
    auto titleValues = filter.from(testData)
        .where("$.store.book[*].title")
        .values();
    assert(titleValues.size() == 4);
    std::cout << "✓ Value extraction: " << titleValues.size() << " titles" << std::endl;

    // Test orderBy (sort by price ascending)
    auto sortedBooks = filter.from(testData)
        .where("$.store.book[*]")
        .orderBy("price", true)
        .execute();
    assert(sortedBooks.size() == 4);
    double prevPrice = 0.0;
    for (size_t i = 0; i < sortedBooks.size(); ++i) {
        auto priceVal = filter.selectFirst(*sortedBooks[i].value, "$.price");
        if (!priceVal || !priceVal->isNumber()) continue; // 跳过无price字段
        double price = priceVal->toDouble();
        if (i > 0 && prevPrice > 0.0) assert(price >= prevPrice);
        prevPrice = price;
    }
    std::cout << "✓ orderBy(price) ascending works" << std::endl;

    // Test orderBy (sort by price descending)
    auto sortedBooksDesc = filter.from(testData)
        .where("$.store.book[*]")
        .orderBy("price", false)
        .execute();
    assert(sortedBooksDesc.size() == 4);
    prevPrice = 1e9;
    for (size_t i = 0; i < sortedBooksDesc.size(); ++i) {
        auto priceVal = filter.selectFirst(*sortedBooksDesc[i].value, "$.price");
        if (!priceVal || !priceVal->isNumber()) continue;
        double price = priceVal->toDouble();
        if (i > 0 && prevPrice < 1e9) assert(price <= prevPrice);
        prevPrice = price;
    }
    std::cout << "✓ orderBy(price) descending works" << std::endl;

    // Test groupBy (group by category)
    auto groupedBooks = filter.from(testData)
        .where("$.store.book[*]")
        .groupBy("category")
        .executeGrouped();
    std::cout << "GroupBy(category) results:" << std::endl;
    for (const auto& kv : groupedBooks) {
        std::cout << "  key='" << kv.first << "' count=" << kv.second.size() << std::endl;
    }
    assert(groupedBooks["fiction"].size() == 3);
    assert(groupedBooks["reference"].size() == 1);
    std::cout << "✓ groupBy(category) works: fiction=" << groupedBooks["fiction"].size() << ", reference=" << groupedBooks["reference"].size() << std::endl;
}

void testBatchOperations(const JsonValue& testData, JsonFilter& filter) {
    std::cout << "\n=== Batch Operation Tests ===" << std::endl;
    
    // Test batch query
    std::vector<std::string> queries = {
        "$.store.book[*].title",
        "$.store.book[*].author",
        "$.store.book[*].price",
        "$.store.book[*].isbn"
    };
    
    auto batchResults = filter.batchQuery(testData, queries);
    assert(batchResults.size() == 4);
    assert(batchResults[0].size() == 4); // titles
    assert(batchResults[1].size() == 4); // authors
    assert(batchResults[2].size() == 4); // prices
    assert(batchResults[3].size() == 2); // ISBNs (only 2 books have ISBN)
    
    std::cout << "✓ Batch query results:" << std::endl;
    for (size_t i = 0; i < queries.size(); ++i) {
        std::cout << "  " << queries[i] << ": " << batchResults[i].size() << " results" << std::endl;
    }
    
    // Test transform operation
    auto priceResults = filter.query(testData, "$.store.book[*].price");
    auto transformedPrices = filter.transform(priceResults,
        [](const JsonValue& value, const std::string& path) -> JsonValue {
            return JsonValue("$" + std::to_string(value.toDouble()));
        }
    );
    assert(transformedPrices.size() == 4);
    std::cout << "✓ Transform operation: " << transformedPrices.size() << " transformed prices" << std::endl;
}

void testConvenienceFunctions(const JsonValue& testData) {
    std::cout << "\n=== Convenience Function Tests ===" << std::endl;
    
    // Test global convenience functions
    assert(query::exists(testData, "$.store.bicycle"));
    std::cout << "✓ Global exists function" << std::endl;
    
    auto firstBook = query::first(testData, "$.store.book[0]");
    assert(firstBook != nullptr);
    std::cout << "✓ Global first function" << std::endl;
    
    auto allAuthors = query::all(testData, "$.store.book[*].author");
    assert(allAuthors.size() == 4);
    std::cout << "✓ Global all function: " << allAuthors.size() << " authors" << std::endl;
    
    auto authorValues = query::values(testData, "$.store.book[*].author");
    assert(authorValues.size() == 4);
    std::cout << "✓ Global values function: " << authorValues.size() << " author values" << std::endl;
    
    // Test chain convenience functions
    auto chainResults = query::from(testData)
        .where("$.store.book[*]")
        .where(JsonFilter::Filters::hasProperty("category"))
        .values();
    assert(chainResults.size() == 4);
    std::cout << "✓ Chain convenience function: " << chainResults.size() << " books with category" << std::endl;
}

void testBackwardCompatibility(const JsonValue& testData) {
    std::cout << "\n=== Backward Compatibility Tests ===" << std::endl;
    
    // Test original JsonValue methods still work
    assert(testData.pathExists("$.store.book[0].title"));
    std::cout << "✓ JsonValue::pathExists still works" << std::endl;
    
    auto firstBookTitle = testData.selectFirst("$.store.book[0].title");
    assert(firstBookTitle != nullptr);
    assert(firstBookTitle->toString() == "Sayings of the Century");
    std::cout << "✓ JsonValue::selectFirst still works" << std::endl;
    
    auto allBookTitles = testData.selectAll("$.store.book[*].title");
    assert(allBookTitles.size() == 4);
    std::cout << "✓ JsonValue::selectAll still works: " << allBookTitles.size() << " titles" << std::endl;
    
    auto titleValues = testData.selectValues("$.store.book[*].title");
    assert(titleValues.size() == 4);
    std::cout << "✓ JsonValue::selectValues still works: " << titleValues.size() << " title values" << std::endl;
}

void testStrictEdgeCases(const JsonValue& testData, JsonFilter& filter) {
    std::cout << "\n=== Strict Edge Case Tests ===" << std::endl;

    // 1. 非法路径测试
    assert(!filter.pathExists(testData, "$.store.book[100]")); // 越界
    assert(!filter.pathExists(testData, "$.store.book[*].nonexistent")); // 不存在字段

    // 2. 类型校验
    auto books = filter.from(testData).where("$.store.book[*]").execute();
    for (const auto& result : books) {
        assert(result.value->isObject());
        auto priceVal = filter.selectFirst(*result.value, "$.price");
        assert(priceVal != nullptr);
        assert(priceVal->isNumber());
    }

    // 3. 排序健壮性（所有price都为double且有序）
    auto sortedBooks = filter.from(testData).where("$.store.book[*]").orderBy("price", true).execute();
    double prevPrice = -1e9;
    for (const auto& result : sortedBooks) {
        auto priceVal = filter.selectFirst(*result.value, "$.price");
        assert(priceVal != nullptr);
        double price = priceVal->toDouble();
        assert(price >= prevPrice);
        prevPrice = price;
    }

    // 4. 分组健壮性（所有分组键都存在且分组数量正确）
    auto groupedBooks = filter.from(testData).where("$.store.book[*]").groupBy("category").executeGrouped();
    size_t totalCount = 0;
    for (const auto& kv : groupedBooks) {
        assert(!kv.first.empty());
        for (const auto& item : kv.second) {
            auto catVal = filter.selectFirst(*item.value, "$.category");
            assert(catVal != nullptr);
            assert(catVal->toString() == kv.first);
            totalCount++;
        }
    }
    assert(totalCount == 4);

    // 5. 异常处理（无效JSONPath抛异常或返回空）
    try {
        auto invalid = filter.query(testData, "$.store.book[0].price.nonexistent");
        assert(invalid.empty());
    } catch (...) {
        std::cout << "Caught exception for invalid path as expected." << std::endl;
    }

    // 6. 空数组/空对象处理
    JsonValue emptyArr = JsonValue::parse("[]");
    assert(filter.selectAll(emptyArr, "$[*]").empty());
    JsonValue emptyObj = JsonValue::parse("{}");
    assert(filter.selectAll(emptyObj, "$.*").empty());

    std::cout << "✓ Strict edge case tests passed" << std::endl;
}

void testSmallJsonSample(JsonFilter& filter) {
    std::cout << "\n=== Small JSON Sample Test ===" << std::endl;
    std::string smallJson = R"({"a":1,"b":2,"c":{"d":3}})";
    JsonValue val = JsonValue::parse(smallJson);
    assert(filter.pathExists(val, "$.a"));
    assert(filter.pathExists(val, "$.c.d"));
    auto result = filter.query(val, "$.c.d");
    assert(result.size() == 1 && result[0].value->toDouble() == 3);
    std::cout << "✓ Small JSON sample test passed" << std::endl;
}

void testLargeJsonSample(JsonFilter& filter) {
    std::cout << "\n=== Large JSON Sample Test ===" << std::endl;
    std::string largeJson = "{\"arr\":[";
    for (int i = 0; i < 10000; ++i) {
        largeJson += std::to_string(i);
        if (i < 9999) largeJson += ",";
    }
    largeJson += "]}";
    JsonValue val = JsonValue::parse(largeJson);
    auto allItems = filter.selectAll(val, "$.arr[*]");
    assert(allItems.size() == 10000);
    // 检查部分数据
    assert(allItems[0]->toDouble() == 0);
    assert(allItems[9999]->toDouble() == 9999);
    std::cout << "✓ Large JSON sample test passed" << std::endl;
}

void testStreamJsonSample(JsonFilter& filter) {
    std::cout << "\n=== Stream JSON Sample Test ===" << std::endl;
    // 模拟流式分块解析
    std::vector<std::string> chunks = {
        "{\"users\":[",
        "{\"id\":1,\"name\":\"Alice\"},",
        "{\"id\":2,\"name\":\"Bob\"},",
        "{\"id\":3,\"name\":\"Carol\"}",
        "]}"
    };
    std::string fullJson;
    for (const auto& chunk : chunks) fullJson += chunk;
    JsonValue val = JsonValue::parse(fullJson);
    auto allUsers = filter.selectAll(val, "$.users[*]");
    assert(allUsers.size() == 3);
    for (const auto& user : allUsers) {
        assert(user->isObject());
        auto idVal = filter.selectFirst(*user, "$.id");
        auto nameVal = filter.selectFirst(*user, "$.name");
        assert(idVal && nameVal);
    }
    std::cout << "✓ Stream JSON sample test passed" << std::endl;
}

void testComplexMultilineString(JsonFilter& filter) {
    std::cout << "\n=== Complex Multiline String Test ===" << std::endl;
    std::string json = R"({
        "single_line": "abc",
        "escaped_newline": "line1\\nline2",
        "actual_newline": "line1
    line2
line3",
        "mixed": "first\\nsecond
third\\tend",
        "array": [
            "item1
item2",
            "item3\\nitem4"
        ],
        "object": {
            "desc": "start
middle\\nend"
        }
    })";
    JsonValue val = JsonValue::parse(json);

    // 单行字符串
    auto single = filter.selectFirst(val, "$.single_line");
    assert(single && single->toString() == "abc");

    // 转义换行
    auto esc = filter.selectFirst(val, "$.escaped_newline");
    assert(esc && esc->toString().find("\\n") != std::string::npos);

    // 实际换行
    auto act = filter.selectFirst(val, "$.actual_newline");
    std::string actStr = act ? act->toString() : "";
    assert(actStr.find("line1") != std::string::npos && actStr.find("line2") != std::string::npos && actStr.find("line3") != std::string::npos);
    assert(actStr.find('\n') != std::string::npos);

    // 混合内容
    auto mix = filter.selectFirst(val, "$.mixed");
    std::string mixStr = mix ? mix->toString() : "";
    assert(mixStr.find("first") != std::string::npos && mixStr.find("second") != std::string::npos && mixStr.find("third") != std::string::npos);

    // 数组中的多行字符串
    auto arr = filter.selectAll(val, "$.array[*]");
    assert(arr.size() == 2);
    assert(arr[0]->toString().find("item1") != std::string::npos && arr[0]->toString().find("item2") != std::string::npos);
    assert(arr[1]->toString().find("\\n") != std::string::npos);

    // 对象中的多行字符串
    auto obj = filter.selectFirst(val, "$.object.desc");
    std::string objStr = obj ? obj->toString() : "";
    assert(objStr.find("start") != std::string::npos && objStr.find("middle") != std::string::npos && objStr.find("end") != std::string::npos);

    std::cout << "✓ Complex multiline string test passed" << std::endl;
}

int main() {
    // Create test JSON data
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

    std::cout << "=== JsonFilter Comprehensive Test Suite ===" << std::endl;

    JsonFilter filter = JsonFilter::createDefault();

    try {
        testBasicPaths(testData, filter);
        testWildcardPaths(testData, filter);
        testComplexQueries(testData, filter);
        testCustomFilters(testData, filter);
        testChainQueries(testData, filter);
        testBatchOperations(testData, filter);
        testConvenienceFunctions(testData);
        testBackwardCompatibility(testData);
        testStrictEdgeCases(testData, filter);
        testSmallJsonSample(filter);
        testLargeJsonSample(filter);
        testStreamJsonSample(filter);
        testComplexMultilineString(filter);

        std::cout << "\n🎉 All tests passed! JsonFilter is working 100% correctly!" << std::endl;
        std::cout << "\n=== Summary ===" << std::endl;
        std::cout << "✅ Basic path queries work correctly" << std::endl;
        std::cout << "✅ Wildcard expressions [*] and * work correctly" << std::endl;
        std::cout << "✅ Complex nested queries work correctly" << std::endl;
        std::cout << "✅ Custom filters work correctly" << std::endl;
        std::cout << "✅ Chain queries work correctly" << std::endl;
        std::cout << "✅ Batch operations work correctly" << std::endl;
        std::cout << "✅ Convenience functions work correctly" << std::endl;
        std::cout << "✅ Backward compatibility is maintained" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
