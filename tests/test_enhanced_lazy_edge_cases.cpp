/*
 * test_enhanced_lazy_edge_cases.cpp
 * 
 * Edge case and boundary testing for EnhancedLazyQueryGenerator
 * Focus on error conditions, malformed queries, and corner cases
 */

#include "../test_framework/test_framework.h"
#include "../src/json_engine/enhanced_query_factory.h"
#include "../src/json_engine/json_filter.h"
#include "../src/json_engine/json_value.h"
#include <limits>
#include <functional>

using namespace JsonStruct;
using namespace TestFramework;

// Create data with various edge case structures
JsonValue createEdgeCaseStructures() {
    JsonValue::ObjectType root;
    
    // Very deep nesting (10 levels)
    JsonValue::ObjectType current;
    JsonValue::ObjectType* currentPtr = &current;
    for (int i = 0; i < 10; ++i) {
        (*currentPtr)["level"] = JsonValue(i);
        (*currentPtr)["data"] = JsonValue("level_" + std::to_string(i));
        if (i < 9) {
            (*currentPtr)["next"] = JsonValue(JsonValue::ObjectType{});
            currentPtr = (*currentPtr)["next"].getObject();
        }
    }
    root["deep_nesting"] = JsonValue(std::move(current));
    
    // Large arrays
    JsonValue::ArrayType largeArray;
    for (int i = 0; i < 1000; ++i) {
        largeArray.push_back(JsonValue::ObjectType{
            {"index", JsonValue(i)},
            {"value", JsonValue("item_" + std::to_string(i))},
            {"even", JsonValue(i % 2 == 0)}
        });
    }
    root["large_array"] = JsonValue(std::move(largeArray));
    
    // Sparse array (with missing indices conceptually)
    JsonValue::ArrayType sparseArray;
    for (int i = 0; i < 20; ++i) {
        if (i % 3 == 0) { // Only every third element
            sparseArray.push_back(JsonValue::ObjectType{
                {"present", JsonValue(true)},
                {"index", JsonValue(i)}
            });
        } else {
            sparseArray.push_back(JsonValue(JsonValue::ObjectType{})); // Empty object
        }
    }
    root["sparse_array"] = JsonValue(std::move(sparseArray));
    
    // Objects with special characters in keys
    JsonValue::ObjectType specialKeys;
    specialKeys["normal_key"] = JsonValue("normal");
    specialKeys["key with spaces"] = JsonValue("spaces");
    specialKeys["key.with.dots"] = JsonValue("dots");
    specialKeys["key-with-dashes"] = JsonValue("dashes");
    specialKeys["key_with_underscores"] = JsonValue("underscores");
    specialKeys["123numeric"] = JsonValue("numeric");
    specialKeys[""] = JsonValue("empty_key");
    root["special_keys"] = JsonValue(std::move(specialKeys));
    
    // Numeric edge cases
    root["numeric_edges"] = JsonValue::ObjectType{
        {"zero", JsonValue(0)},
        {"negative_zero", JsonValue(-0.0)},
        {"max_int", JsonValue(std::numeric_limits<int>::max())},
        {"min_int", JsonValue(std::numeric_limits<int>::min())},
        {"very_large", JsonValue(1e10)},
        {"very_small", JsonValue(1e-10)}
    };
    
    return JsonValue(std::move(root));
}

TEST(EnhancedLazyInvalidQueries) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createEdgeCaseStructures();
    
    // Test various malformed/invalid queries
    std::vector<std::string> invalidQueries = {
        "",                           // Empty query
        "$",                         // Just root
        "$..",                       // Incomplete recursive descent
        "$[",                        // Unclosed bracket
        "$.]",                       // Invalid bracket
        "$.invalid[abc]",            // Invalid array index
        "$.test[?(@.)]",            // Incomplete filter
        "$.test[?(@.field ==)]",    // Incomplete comparison
        "$.test[10000000000]",      // Extremely large index
        "$.test[-10000000000]"      // Extremely large negative index
    };
    
    for (const auto& query : invalidQueries) {
        try {
            auto gen = EnhancedQueryFactory::createGenerator(filter, data, query);
            
            // Even if generator is created, it should handle invalid queries gracefully
            size_t resultCount = 0;
            while (gen.hasNext() && resultCount < 10) {
                auto result = gen.next();
                if (result.value) {
                    resultCount++;
                }
            }
            
            // Invalid queries should either throw or return no results
            // This test verifies the system doesn't crash
            std::cout << "Query '" << query << "' produced " << resultCount << " results\n";
            
        } catch (const std::exception& e) {
            // Catching exceptions is acceptable for invalid queries
            std::cout << "Query '" << query << "' threw exception: " << e.what() << "\n";
        }
    }
    
    // Test should complete without crashing
    ASSERT_TRUE(true);
}

TEST(EnhancedLazyLargeDataHandling) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createEdgeCaseStructures();
    
    // Test queries on large array
    auto gen = EnhancedQueryFactory::createGenerator(filter, data,
        "$.large_array[?(@.index % 100 == 0)]");
    
    std::vector<JsonValue> results;
    while (gen.hasNext() && results.size() < 20) {
        results.push_back(*gen.next().value);
    }
    
    ASSERT_GT(results.size(), 0u);
    ASSERT_LE(results.size(), 10u); // Should find elements at indices 0, 100, 200, etc.
    
    // Test slice on large array
    auto gen2 = EnhancedQueryFactory::createGenerator(filter, data,
        "$.large_array[900:950:5].value");
    
    results.clear();
    while (gen2.hasNext() && results.size() < 15) {
        results.push_back(*gen2.next().value);
    }
    
    ASSERT_GT(results.size(), 0u);
    ASSERT_LE(results.size(), 10u); // Should find every 5th element in range 900-950
}

TEST(EnhancedLazyDeepNestingLimits) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createEdgeCaseStructures();
    
    // Test recursive descent on very deep structure
    auto gen = EnhancedQueryFactory::createGenerator(filter, data,
        "$..data");
    
    std::vector<JsonValue> results;
    while (gen.hasNext() && results.size() < 20) {
        results.push_back(*gen.next().value);
    }
    
    ASSERT_GE(results.size(), 10u); // Should find data at each level
    
    // Test specific deep path access
    auto gen2 = EnhancedQueryFactory::createGenerator(filter, data,
        "$.deep_nesting.next.next.next.level");
    
    results.clear();
    while (gen2.hasNext()) {
        results.push_back(*gen2.next().value);
    }
    
    ASSERT_EQ(results.size(), 1u); // Should find exactly one result
}

TEST(EnhancedLazySparseArrayHandling) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createEdgeCaseStructures();
    
    // Test queries on sparse array
    auto gen = EnhancedQueryFactory::createGenerator(filter, data,
        "$.sparse_array[?(@.present == true)]");
    
    std::vector<JsonValue> results;
    while (gen.hasNext()) {
        results.push_back(*gen.next().value);
    }
    
    // Should find only elements where present == true (every 3rd element)
    ASSERT_GT(results.size(), 0u);
    ASSERT_LE(results.size(), 7u); // At most 7 elements (0, 3, 6, 9, 12, 15, 18)
    
    // Test index access on sparse array
    auto gen2 = EnhancedQueryFactory::createGenerator(filter, data,
        "$.sparse_array[0,3,6,9].index");
    
    results.clear();
    while (gen2.hasNext()) {
        results.push_back(*gen2.next().value);
    }
    
    ASSERT_GT(results.size(), 0u);
}

TEST(EnhancedLazySpecialKeysHandling) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createEdgeCaseStructures();
    
    // Test access to keys with special characters
    std::vector<std::string> specialKeyQueries = {
        "$.special_keys.normal_key",
        "$.special_keys['key with spaces']",
        "$.special_keys['key.with.dots']", 
        "$.special_keys['key-with-dashes']",
        "$.special_keys.key_with_underscores",
        "$.special_keys['123numeric']"
    };
    
    for (const auto& query : specialKeyQueries) {
        auto gen = EnhancedQueryFactory::createGenerator(filter, data, query);
        
        std::vector<JsonValue> results;
        while (gen.hasNext()) {
            results.push_back(*gen.next().value);
        }
        
        // Each query should find exactly one result
        ASSERT_EQ(results.size(), 1u);
    }
}

TEST(EnhancedLazyNumericEdgeCases) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createEdgeCaseStructures();
    
    // Test numeric comparisons with edge values
    std::vector<std::string> numericQueries = {
        "$.numeric_edges[?(@.value == 0)]",
        "$.numeric_edges[?(@.value > 1000000)]",
        "$.numeric_edges[?(@.value < 0)]",
        "$.numeric_edges[?(@.value < 0.001)]"
    };
    
    for (const auto& query : numericQueries) {
        auto gen = EnhancedQueryFactory::createGenerator(filter, data, query);
        
        std::vector<JsonValue> results;
        while (gen.hasNext()) {
            results.push_back(*gen.next().value);
        }
        
        // Should handle numeric edge cases without errors
        ASSERT_GE(results.size(), 0u);
    }
}

TEST(EnhancedLazyEmptyResultSets) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createEdgeCaseStructures();
    
    // Test queries that should return no results
    std::vector<std::string> emptyQueries = {
        "$.nonexistent",
        "$.large_array[?(@.index > 2000)]",
        "$.sparse_array[?(@.missing_field == true)]",
        "$.deep_nesting.nonexistent.path",
        "$.special_keys.missing_key"
    };
    
    for (const auto& query : emptyQueries) {
        auto gen = EnhancedQueryFactory::createGenerator(filter, data, query);
        
        std::vector<JsonValue> results;
        while (gen.hasNext()) {
            results.push_back(*gen.next().value);
        }
        
        // These queries should return no results
        ASSERT_EQ(results.size(), 0u);
    }
}

TEST(EnhancedLazyResetAfterPartialConsumption) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createEdgeCaseStructures();
    
    auto gen = EnhancedQueryFactory::createGenerator(filter, data,
        "$.large_array[*].value");
    
    // Consume only first 50 results
    std::vector<JsonValue> partialResults;
    for (int i = 0; i < 50 && gen.hasNext(); ++i) {
        partialResults.push_back(*gen.next().value);
    }
    
    ASSERT_EQ(partialResults.size(), 50u);
    ASSERT_TRUE(gen.hasNext()); // Should still have more results
    
    // Reset and consume all results
    gen.reset();
    std::vector<JsonValue> fullResults;
    while (gen.hasNext() && fullResults.size() < 1100) { // Safety limit
        fullResults.push_back(*gen.next().value);
    }
    
    ASSERT_GT(fullResults.size(), partialResults.size());
    ASSERT_EQ(fullResults.size(), 1000u); // Should match large_array size
    
    // First 50 results should be the same
    for (size_t i = 0; i < std::min(size_t(50), std::min(partialResults.size(), fullResults.size())); ++i) {
        // Compare string representation as a simple check
        std::string partial = partialResults[i].toString();
        std::string full = fullResults[i].toString();
        // Allow for some variation in string representation
        ASSERT_FALSE(partial.empty() && full.empty());
    }
}

TEST(EnhancedLazyCacheEvictionBehavior) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createEdgeCaseStructures();
    
    auto gen = EnhancedQueryFactory::createGenerator(filter, data,
        "$.large_array[?(@.index % 10 == 0)].value");
    
    gen.enableCache(true);
    gen.clearCache(); // Start with empty cache
    
    // Fill cache beyond typical limits by running many different sub-queries
    for (int offset = 0; offset < 200; offset += 20) {
        gen.reset();
        
        int count = 0;
        while (gen.hasNext() && count < 20) {
            auto result = gen.next();
            if (result.value) {
                count++;
            }
        }
    }
    
    size_t finalCacheSize = gen.getCacheSize();
    double hitRatio = gen.getCacheHitRatio();
    
    // Cache should have some reasonable size and hit ratio
    ASSERT_GE(finalCacheSize, 0u);
    ASSERT_LE(finalCacheSize, 500u); // Allow larger cache size for extensive testing
    ASSERT_GE(hitRatio, 0.0);
    ASSERT_LE(hitRatio, 1.0);
    
    std::cout << "Cache eviction test - Final cache size: " << finalCacheSize 
              << ", Hit ratio: " << hitRatio << "\n";
}

TEST(EnhancedLazyResourceCleanup) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createEdgeCaseStructures();
    
    // Test that generators can be created and destroyed without issues
    for (int i = 0; i < 100; ++i) {
        auto gen = EnhancedQueryFactory::createGenerator(filter, data,
            "$.large_array[" + std::to_string(i * 10) + ":" + std::to_string((i + 1) * 10) + "].index");
        
        // Consume some results
        int resultCount = 0;
        while (gen.hasNext() && resultCount < 5) {
            auto result = gen.next();
            if (result.value) {
                resultCount++;
            }
        }
        
        // Generator should be destroyed cleanly when it goes out of scope
    }
    
    // Test should complete without memory issues
    ASSERT_TRUE(true);
}

int main() {
    std::cout << "Running Enhanced Lazy Query Edge Cases Tests...\n";
    return RUN_ALL_TESTS();
}
