#include "../src/json_engine/json_value.h"
#include "../src/json_engine/json_query_generator.h"
#include <iostream>
#include <vector>
#include <memory>
#include <cassert>

using namespace JsonStruct;

// Simple test framework replacement
#define EXPECT_EQ(a, b) assert((a) == (b))
#define EXPECT_NE(a, b) assert((a) != (b))
#define EXPECT_TRUE(a) assert(a)
#define EXPECT_FALSE(a) assert(!(a))
#define EXPECT_LT(a, b) assert((a) < (b))

void testBasicStreamQuery() {
    std::cout << "Testing basic stream query..." << std::endl;
    
    // Create test JSON structure
    JsonValue testJson = JsonValue::object({
        {"users", JsonValue::array({
            JsonValue::object({
                {"id", JsonValue(1)},
                {"name", JsonValue("Alice")},
                {"age", JsonValue(25)},
                {"active", JsonValue(true)}
            }),
            JsonValue::object({
                {"id", JsonValue(2)},
                {"name", JsonValue("Bob")},
                {"age", JsonValue(30)},
                {"active", JsonValue(false)}
            }),
            JsonValue::object({
                {"id", JsonValue(3)},
                {"name", JsonValue("Charlie")},
                {"age", JsonValue(35)},
                {"active", JsonValue(true)}
            }),
            JsonValue::object({
                {"id", JsonValue(4)},
                {"name", JsonValue("Diana")},
                {"age", JsonValue(28)},
                {"active", JsonValue(true)}
            })
        })},
        {"metadata", JsonValue::object({
            {"total", JsonValue(4)},
            {"version", JsonValue("1.0")}
        })}
    });
    
    auto generator = JsonStreamingQuery::createGenerator(testJson, "$.users[*].name");
    
    std::vector<std::string> names;
    for (auto it = generator.begin(); it != generator.end(); ++it) {
        if (auto name = it->first->getString()) {
            names.push_back(std::string(*name));
        }
    }
    
    EXPECT_EQ(names.size(), 4);
    EXPECT_EQ(names[0], "Alice");
    EXPECT_EQ(names[1], "Bob");
    EXPECT_EQ(names[2], "Charlie");
    EXPECT_EQ(names[3], "Diana");
    
    std::cout << "Basic stream query test passed!" << std::endl;
}

void testLimitedResults() {
    std::cout << "Testing limited results..." << std::endl;
    
    JsonValue testJson = JsonValue::object({
        {"data", JsonValue::array({
            JsonValue(1), JsonValue(2), JsonValue(3), JsonValue(4), JsonValue(5)
        })}
    });
    
    JsonQueryGenerator::GeneratorOptions options;
    options.maxResults = 2;
    
    auto generator = JsonStreamingQuery::createGenerator(testJson, "$.data[*]", options);
    
    size_t count = 0;
    for (auto it = generator.begin(); it != generator.end(); ++it) {
        ++count;
    }
    
    EXPECT_EQ(count, 2);
    EXPECT_EQ(generator.getTotalGenerated(), 2);
    
    std::cout << "Limited results test passed!" << std::endl;
}

void testLazyQuery() {
    std::cout << "Testing lazy query..." << std::endl;
    
    JsonValue testJson = JsonValue::object({
        {"numbers", JsonValue::array({
            JsonValue(1), JsonValue(2), JsonValue(3), JsonValue(4), JsonValue(5)
        })}
    });
    
    std::vector<int> evenNumbers;
    
    size_t processed = JsonStreamingQuery::lazyQuery(testJson, "$.numbers[*]", 
        [&](const JsonValue* value, const std::string& path) -> bool {
            if (auto num = value->getInteger()) {
                if (*num % 2 == 0) {
                    evenNumbers.push_back(static_cast<int>(*num));
                }
            }
            return true;  // Continue processing
        });
    
    EXPECT_EQ(processed, 5);  // All numbers processed
    EXPECT_EQ(evenNumbers.size(), 2);  // 2 even numbers
    EXPECT_EQ(evenNumbers[0], 2);
    EXPECT_EQ(evenNumbers[1], 4);
    
    std::cout << "Lazy query test passed!" << std::endl;
}

void testFindFirst() {
    std::cout << "Testing find first..." << std::endl;
    
    JsonValue testJson = JsonValue::object({
        {"items", JsonValue::array({
            JsonValue::object({{"value", JsonValue(10)}}),
            JsonValue::object({{"value", JsonValue(20)}}),
            JsonValue::object({{"value", JsonValue(30)}}),
            JsonValue::object({{"value", JsonValue(40)}})
        })}
    });
    
    auto result = JsonStreamingQuery::findFirst(testJson, "$.items[*]");
    
    EXPECT_TRUE(result.has_value());
    
    if (result) {
        auto value = (*result->first)["value"].getInteger();
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(*value, 10);  // First item
    }
    
    std::cout << "Find first test passed!" << std::endl;
}

void testCountMatches() {
    std::cout << "Testing count matches..." << std::endl;
    
    JsonValue testJson = JsonValue::object({
        {"items", JsonValue::array({
            JsonValue(1), JsonValue(2), JsonValue(3), JsonValue(4), JsonValue(5)
        })}
    });
    
    size_t count = JsonStreamingQuery::countMatches(testJson, "$.items[*]");
    EXPECT_EQ(count, 5);
    
    size_t limitedCount = JsonStreamingQuery::countMatches(testJson, "$.items[*]", 3);
    EXPECT_EQ(limitedCount, 3);
    
    std::cout << "Count matches test passed!" << std::endl;
}

void testBatchProcessing() {
    std::cout << "Testing batch processing..." << std::endl;
    
    JsonValue testJson = JsonValue::object({
        {"data", JsonValue::array({
            JsonValue(1), JsonValue(2), JsonValue(3), JsonValue(4), JsonValue(5), JsonValue(6)
        })}
    });
    
    JsonQueryGenerator::GeneratorOptions options;
    options.batchSize = 2;
    
    auto generator = JsonStreamingQuery::createGenerator(testJson, "$.data[*]", options);
    
    std::vector<std::vector<int>> batches;
    
    while (generator.hasMore()) {
        auto batch = generator.takeBatch(2);
        if (batch.empty()) break;
        
        std::vector<int> values;
        for (const auto& [value, path] : batch) {
            if (auto num = value->getInteger()) {
                values.push_back(static_cast<int>(*num));
            }
        }
        batches.push_back(values);
    }
    
    EXPECT_EQ(batches.size(), 3);  // 6 items in batches of 2
    EXPECT_EQ(batches[0].size(), 2);
    EXPECT_EQ(batches[1].size(), 2);
    EXPECT_EQ(batches[2].size(), 2);
    
    std::cout << "Batch processing test passed!" << std::endl;
}

int main() {
    std::cout << "=== Streaming Query Tests ===" << std::endl;
    
    try {
        testBasicStreamQuery();
        testLimitedResults();
        testLazyQuery();
        testFindFirst();
        testCountMatches();
        testBatchProcessing();
        
        std::cout << std::endl << "All tests passed successfully!" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}
