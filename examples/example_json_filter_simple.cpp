#include <iostream>
#include <string>
#include "json_value.h"
#include "json_filter.h"

using namespace JsonStruct;
using namespace JsonStruct::filter_types;

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

    std::cout << "=== JsonFilter Usage Example ===" << std::endl;

    // Basic usage
    std::cout << "\n--- Basic Query Methods ---" << std::endl;
    
    // Create filter instance
    JsonFilter filter = JsonFilter::createDefault();
    
    // 1. Path existence check
    bool exists = filter.pathExists(testData, "$.store.book[0].title");
    std::cout << "Path exists: " << (exists ? "Yes" : "No") << std::endl;

    // 2. Select first match
    const JsonValue* firstBook = filter.selectFirst(testData, "$.store.book[0]");
    if (firstBook) {
        std::cout << "First book: " << firstBook->dump(2) << std::endl;
    }

    // 3. Select all matches
    auto allBooks = filter.selectAll(testData, "$.store.book[*]");
    std::cout << "Found " << allBooks.size() << " books" << std::endl;

    // 4. Get value copies
    auto bookTitles = filter.selectValues(testData, "$.store.book[*].title");
    std::cout << "Book titles count: " << bookTitles.size() << std::endl;

    // Advanced usage
    std::cout << "\n--- Advanced Query Methods ---" << std::endl;

    // 1. Detailed query
    auto queryResults = filter.query(testData, "$.store.book[*].price");
    std::cout << "Price query results:" << std::endl;
    for (const auto& result : queryResults) {
        std::cout << "  Path: " << result.path << ", Value: " << result.value->dump() << std::endl;
    }

    // 2. Custom filter
    auto expensiveBooks = filter.queryWithFilter(testData, 
        JsonFilter::Filters::byNumberRange(10.0, 100.0)
    );
    std::cout << "Items with price 10-100: " << expensiveBooks.size() << std::endl;

    // 3. Regex filtering
    auto priceResults = filter.queryWithRegex(testData, R"(.*\.price$)");
    std::cout << "All price fields count: " << priceResults.size() << std::endl;

    // Chain query API
    std::cout << "\n--- Chain Query API ---" << std::endl;

    auto fictionBooks = filter.from(testData)
        .where("$.store.book[*]")
        .where(JsonFilter::Filters::hasProperty("category"))
        .limit(2)
        .execute();
    
    std::cout << "Found books (limit 2): " << fictionBooks.size() << std::endl;

    // Statistics query
    size_t bookCount = filter.from(testData)
        .where("$.store.book[*]")
        .count();
    std::cout << "Total books: " << bookCount << std::endl;

    bool hasExpensiveBooks = filter.from(testData)
        .where("$.store.book[*].price")
        .any();
    std::cout << "Has books with price: " << (hasExpensiveBooks ? "Yes" : "No") << std::endl;

    // Batch operations
    std::cout << "\n--- Batch Operations ---" << std::endl;

    // Batch query
    std::vector<std::string> queries = {
        "$.store.book[*].title",
        "$.store.book[*].author",
        "$.store.book[*].price"
    };

    auto batchResults = filter.batchQuery(testData, queries);
    std::cout << "Batch query results:" << std::endl;
    for (size_t i = 0; i < queries.size(); ++i) {
        std::cout << "  Query '" << queries[i] << "': " << batchResults[i].size() << " results" << std::endl;
    }

    // Transform operation
    auto titles = filter.transform(queryResults, 
        [](const JsonValue& value, const std::string& path) -> JsonValue {
            return JsonValue("Price: " + value.toString());
        }
    );
    std::cout << "Transformed results count: " << titles.size() << std::endl;

    // Different filter configurations
    std::cout << "\n--- Different Filter Configurations ---" << std::endl;

    // High performance filter (with caching)
    JsonFilter highPerfFilter = JsonFilter::createHighPerformance();
    auto cachedResult1 = highPerfFilter.selectAll(testData, "$.store.book[*].title");
    auto cachedResult2 = highPerfFilter.selectAll(testData, "$.store.book[*].title"); // Should use cache
    std::cout << "High performance filter result: " << cachedResult1.size() << " items" << std::endl;

    // Strict mode filter (disable advanced features)
    JsonFilter strictFilter = JsonFilter::createStrict();
    auto strictResult = strictFilter.selectFirst(testData, "$.store.book[0].title");
    std::cout << "Strict mode query result: " << (strictResult ? "Success" : "Failed") << std::endl;

    // Predefined filters example
    std::cout << "\n--- Predefined Filters ---" << std::endl;

    // Filter by type
    auto stringValues = filter.queryWithFilter(testData, JsonFilter::Filters::byType(String));
    std::cout << "String type values count: " << stringValues.size() << std::endl;

    // Filter by string value
    auto fictionFilter = filter.queryWithFilter(testData, JsonFilter::Filters::byString("fiction", true));
    std::cout << "Items with value 'fiction': " << fictionFilter.size() << std::endl;

    // Non-empty filter
    auto nonEmptyValues = filter.queryWithFilter(testData, JsonFilter::Filters::isNotEmpty());
    std::cout << "Non-empty values count: " << nonEmptyValues.size() << std::endl;

    // Convenience functions
    std::cout << "\n--- Convenience Functions ---" << std::endl;

    // Global convenience functions
    bool quickExists = query::exists(testData, "$.store.bicycle.color");
    std::cout << "Quick existence check: " << (quickExists ? "Yes" : "No") << std::endl;

    auto quickFirst = query::first(testData, "$.store.bicycle.price");
    if (quickFirst) {
        std::cout << "Bicycle price: " << quickFirst->dump() << std::endl;
    }

    auto quickAll = query::all(testData, "$.store.book[*].author");
    std::cout << "All authors count: " << quickAll.size() << std::endl;

    // Chain convenience function
    auto quickChain = query::from(testData)
        .where("$.store.book[*]")
        .where(JsonFilter::Filters::hasProperty("isbn"))
        .values();
    std::cout << "Books with ISBN count: " << quickChain.size() << std::endl;

    std::cout << "\n=== Example Complete ===" << std::endl;

    return 0;
}
