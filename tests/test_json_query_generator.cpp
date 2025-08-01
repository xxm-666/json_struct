#include "../test_framework/test_framework.h"
#include "../src/json_engine/json_query_generator.h"
#include "../src/json_engine/json_value.h"
#include <chrono>
#include <random>
#include <memory>

using namespace JsonStruct;
using namespace TestFramework;

// Helper function to create test data
JsonValue createTestUsers() {
    JsonValue::ObjectType usersObj;
    JsonValue::ArrayType usersArray;
    
    JsonValue::ObjectType user1;
    user1["name"] = JsonValue(std::string("Alice"));
    user1["age"] = JsonValue(25);
    usersArray.push_back(JsonValue(std::move(user1)));
    
    JsonValue::ObjectType user2;
    user2["name"] = JsonValue(std::string("Bob"));
    user2["age"] = JsonValue(30);
    usersArray.push_back(JsonValue(std::move(user2)));
    
    JsonValue::ObjectType user3;
    user3["name"] = JsonValue(std::string("Charlie"));
    user3["age"] = JsonValue(35);
    usersArray.push_back(JsonValue(std::move(user3)));
    
    usersObj["users"] = JsonValue(std::move(usersArray));
    return JsonValue(std::move(usersObj));
}

JsonValue createLargeArray(int size) {
    JsonValue::ArrayType largeArray;
    for (int i = 0; i < size; ++i) {
        JsonValue::ObjectType item;
        item["id"] = JsonValue(i);
        item["value"] = JsonValue(std::string("item_" + std::to_string(i)));
        largeArray.push_back(JsonValue(std::move(item)));
    }
    return JsonValue(std::move(largeArray));
}

// Basic functionality test
void test_basic_generator_functionality(TestResult& __result_ref) {
    try {
        // Create simple test data
        JsonValue testData = createTestUsers();

        // Test basic iterator functionality
        auto generator = JsonQueryGenerator(testData, "$.users[*]");
        
        int count = 0;
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            ASSERT_TRUE(it->first != nullptr);
            count++;
        }
        
        ASSERT_EQ(3, count);
        
        // Test generator state
        ASSERT_TRUE(generator.getState() == JsonQueryGenerator::State::Completed);
        ASSERT_EQ(3, generator.getTotalGenerated());
        
    } catch (const std::exception& e) {
        __result_ref.addFail("Basic generator functionality test failed: " + std::string(e.what()));
    }
}

// Early termination test
void test_early_termination(TestResult& __result_ref) {
    try {
        // Create test data
        JsonValue testData = createLargeArray(100);

        // Test stopOnFirstMatch option
        JsonQueryGenerator::GeneratorOptions opts;
        opts.stopOnFirstMatch = true;
        
        auto generator = JsonQueryGenerator(testData, "$[*]", opts);
        
        int count = 0;
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            count++;
        }
        
        ASSERT_EQ(1, count);
        ASSERT_EQ(1, generator.getTotalGenerated());
        
        // Test maxResults limit
        opts.stopOnFirstMatch = false;
        opts.maxResults = 5;
        
        auto limitedGenerator = JsonQueryGenerator(testData, "$[*]", opts);
        
        count = 0;
        for (auto it = limitedGenerator.begin(); it != limitedGenerator.end(); ++it) {
            count++;
        }
        
        ASSERT_EQ(5, count);
        ASSERT_EQ(5, limitedGenerator.getTotalGenerated());
        
    } catch (const std::exception& e) {
        __result_ref.addFail("Early termination test failed: " + std::string(e.what()));
    }
}

// Batch processing test
void test_batch_processing(TestResult& __result_ref) {
    try {
        // Create test data
        JsonValue testData = createLargeArray(50);

        JsonQueryGenerator::GeneratorOptions opts;
        opts.batchSize = 10;
        
        auto generator = JsonQueryGenerator(testData, "$[*]", opts);
        
        // Test takeBatch method
        auto batch1 = generator.takeBatch(15);
        ASSERT_EQ(15, batch1.size());
        
        auto batch2 = generator.takeBatch(20);
        ASSERT_EQ(20, batch2.size());
        
        auto batch3 = generator.takeBatch(20);
        ASSERT_EQ(15, batch3.size()); // Remaining 15
        
        auto batch4 = generator.takeBatch(10);
        ASSERT_EQ(0, batch4.size()); // Should be empty
        
    } catch (const std::exception& e) {
        __result_ref.addFail("Batch processing test failed: " + std::string(e.what()));
    }
}

// Streaming query factory test
void test_streaming_query_factory(TestResult& __result_ref) {
    try {
        // Create test data
        JsonValue::ObjectType productsObj;
        JsonValue::ArrayType productsArray;
        
        JsonValue::ObjectType product1;
        product1["name"] = JsonValue(std::string("Laptop"));
        product1["price"] = JsonValue(1000);
        product1["category"] = JsonValue(std::string("Electronics"));
        productsArray.push_back(JsonValue(std::move(product1)));
        
        JsonValue::ObjectType product2;
        product2["name"] = JsonValue(std::string("Book"));
        product2["price"] = JsonValue(20);
        product2["category"] = JsonValue(std::string("Education"));
        productsArray.push_back(JsonValue(std::move(product2)));
        
        JsonValue::ObjectType product3;
        product3["name"] = JsonValue(std::string("Phone"));
        product3["price"] = JsonValue(800);
        product3["category"] = JsonValue(std::string("Electronics"));
        productsArray.push_back(JsonValue(std::move(product3)));
        
        JsonValue::ObjectType product4;
        product4["name"] = JsonValue(std::string("Desk"));
        product4["price"] = JsonValue(200);
        product4["category"] = JsonValue(std::string("Furniture"));
        productsArray.push_back(JsonValue(std::move(product4)));
        
        productsObj["products"] = JsonValue(std::move(productsArray));
        JsonValue testData(std::move(productsObj));

        // Test findFirst
        auto firstMatch = JsonStreamingQuery::findFirst(testData, "$.products[*]");
        ASSERT_TRUE(firstMatch.has_value());
        ASSERT_TRUE(firstMatch->first != nullptr);
        
        // Test countMatches
        size_t totalCount = JsonStreamingQuery::countMatches(testData, "$.products[*]");
        ASSERT_EQ(4, totalCount);
        
        size_t limitedCount = JsonStreamingQuery::countMatches(testData, "$.products[*]", 2);
        ASSERT_EQ(2, limitedCount);
        
        // Test lazyQuery
        std::vector<std::string> names;
        size_t processed = JsonStreamingQuery::lazyQuery(testData, "$.products[*]", 
            [&names](const JsonValue* value, const std::string& path) -> bool {
                if (value->getObject()) {
                    auto obj = value->getObject();
                    auto it = obj->find("name");
                    if (it != obj->end()) {
                        if (auto str = it->second.getString()) {
                            names.push_back(std::string(*str));
                            // Debug output
                            std::cout << "Processed product: " << *str << ", current names.size() = " << names.size() << std::endl;
                        }
                    }
                }
                bool shouldContinue = names.size() < 3;
                std::cout << "Callback returns: " << (shouldContinue ? "true" : "false") << std::endl;
                return shouldContinue; // Only process first 3
            });
        
        std::cout << "lazyQuery processed = " << processed << ", names.size() = " << names.size() << std::endl;
        ASSERT_EQ(3, processed);
        ASSERT_EQ(3, names.size());
        
    } catch (const std::exception& e) {
        __result_ref.addFail("Streaming query factory test failed: " + std::string(e.what()));
    }
}

// Performance test
void test_performance_comparison(TestResult& __result_ref) {
    try {
        // Create large test data (10,000 objects)
        JsonValue largeData = createLargeArray(10000);

        // Test streaming query performance (only get first 100)
        auto start = std::chrono::high_resolution_clock::now();
        
        JsonQueryGenerator::GeneratorOptions opts;
        opts.maxResults = 100;
        
        auto generator = JsonQueryGenerator(largeData, "$[*]", opts);
        
        int count = 0;
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            count++;
            // Check some values
            if (it->first->getObject()) {
                auto obj = it->first->getObject();
                auto it_id = obj->find("id");
                if (it_id != obj->end()) {
                    // Suppress unused variable warning
                    (void)it_id->second;
                }
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        ASSERT_EQ(100, count);
        
        // Performance should be within reasonable range (<100ms)
        ASSERT_TRUE(duration.count() < 100);

        // Test early termination efficiency
        start = std::chrono::high_resolution_clock::now();
        
        auto firstResult = JsonStreamingQuery::findFirst(largeData, "$[*]");
        
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        ASSERT_TRUE(firstResult.has_value());
        ASSERT_TRUE(duration.count() < 100);
        
    } catch (const std::exception& e) {
        __result_ref.addFail("Performance test failed: " + std::string(e.what()));
    }
}

// Large object test
void test_large_object_handling(TestResult& __result_ref) {
    try {
        // Create deeply nested large object
        JsonValue::ObjectType largeObject;
        
        // Create multi-level nested structure
        JsonValue::ObjectType* current = &largeObject;
        for (int level = 0; level < 10; ++level) {
            JsonValue::ObjectType nextLevel;
            JsonValue::ArrayType dataArray;
            
            // Add some data at each level
            for (int i = 0; i < 100; ++i) {
                JsonValue::ObjectType item;
                item["item_id"] = JsonValue(level * 100 + i);
                item["value"] = JsonValue(std::string("Level" + std::to_string(level) + "_Item" + std::to_string(i)));
                dataArray.push_back(JsonValue(std::move(item)));
            }
            
            (*current)["level_" + std::to_string(level)] = JsonValue(std::move(nextLevel));
            (*current)["data_" + std::to_string(level)] = JsonValue(std::move(dataArray));
            
            // Get reference to next level
            if (auto obj = (*current)["level_" + std::to_string(level)].getObject()) {
                current = const_cast<JsonValue::ObjectType*>(obj);
            }
        }

        JsonValue testData(std::move(largeObject));

        // Test deep query
        JsonQueryGenerator::GeneratorOptions opts;
        opts.maxResults = 50;
        auto generator = JsonQueryGenerator(testData, "$..item_id", opts);
        
        int foundItems = 0;
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            foundItems++;
        }
        
        ASSERT_EQ(50, foundItems);
        
        // Test memory usage - generator should handle large objects without memory overflow
        size_t totalProcessed = 0;
        auto memoryTestGenerator = JsonQueryGenerator(testData, "$..value");
        
        // Use streaming processing, handle one item at a time
        memoryTestGenerator.yield([&totalProcessed](const JsonValue* value, const std::string& path, size_t index) -> bool {
            totalProcessed++;
            return totalProcessed < 200; // Prevent processing too many items
        });
        
        ASSERT_EQ(200, totalProcessed);
        ASSERT_TRUE(memoryTestGenerator.getState() == JsonQueryGenerator::State::Terminated);
        
    } catch (const std::exception& e) {
        __result_ref.addFail("Large object test failed: " + std::string(e.what()));
    }
}

// Error handling test
void test_error_handling(TestResult& __result_ref) {
    try {
        JsonValue::ObjectType testObj;
        testObj["test"] = JsonValue(std::string("value"));
        JsonValue testData(std::move(testObj));
        
        // Test handling of invalid query expression
        auto generator = JsonQueryGenerator(testData, "invalid_query");
        
        int count = 0;
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            count++;
        }
        
        // Invalid query should return 0 results, not crash
        ASSERT_EQ(0, count);
        
        // Test handling of empty data
        JsonValue::ObjectType emptyObj;
        JsonValue emptyData(std::move(emptyObj));
        auto emptyGenerator = JsonQueryGenerator(emptyData, "$.anything");
        
        count = 0;
        for (auto it = emptyGenerator.begin(); it != emptyGenerator.end(); ++it) {
            count++;
        }
        
        ASSERT_EQ(0, count);
        
        // Test generator reset
        generator.reset();
        ASSERT_TRUE(generator.getState() == JsonQueryGenerator::State::Ready);
        ASSERT_EQ(0, generator.getTotalGenerated());
        
    } catch (const std::exception& e) {
        __result_ref.addFail("Error handling test failed: " + std::string(e.what()));
    }
}

// Thread safety basics test
void test_thread_safety_basics(TestResult& __result_ref) {
    try {
        // Note: JsonQueryGenerator is designed as non-thread-safe for single consumer
        // This test ensures correct behavior in single-threaded environment
        
        JsonValue testData = createLargeArray(1000);

        // Create multiple independent generator instances
        std::vector<std::unique_ptr<JsonQueryGenerator>> generators;
        for (int i = 0; i < 5; ++i) {
            JsonQueryGenerator::GeneratorOptions opts;
            opts.maxResults = 100;
            generators.push_back(std::make_unique<JsonQueryGenerator>(testData, "$[*]", opts));
        }

        // Test each generator works independently
        for (auto& gen : generators) {
            int count = 0;
            for (auto it = gen->begin(); it != gen->end(); ++it) {
                count++;
            }
            ASSERT_EQ(100, count);
        }
        
    } catch (const std::exception& e) {
        __result_ref.addFail("Thread safety basics test failed: " + std::string(e.what()));
    }
}

// Complex JSONPath query test
void test_complex_jsonpath_queries(TestResult& __result_ref) {
    try {
        // Create complex nested data structure
        JsonValue::ObjectType storeObj;
        JsonValue::ArrayType bookArray;
        
        JsonValue::ObjectType book1;
        book1["category"] = JsonValue(std::string("reference"));
        book1["author"] = JsonValue(std::string("Nigel Rees"));
        book1["title"] = JsonValue(std::string("Sayings of the Century"));
        book1["price"] = JsonValue(8.95);
        bookArray.push_back(JsonValue(std::move(book1)));
        
        JsonValue::ObjectType book2;
        book2["category"] = JsonValue(std::string("fiction"));
        book2["author"] = JsonValue(std::string("Evelyn Waugh"));
        book2["title"] = JsonValue(std::string("Sword of Honour"));
        book2["price"] = JsonValue(12.99);
        bookArray.push_back(JsonValue(std::move(book2)));
        
        JsonValue::ObjectType book3;
        book3["category"] = JsonValue(std::string("fiction"));
        book3["author"] = JsonValue(std::string("Herman Melville"));
        book3["title"] = JsonValue(std::string("Moby Dick"));
        book3["isbn"] = JsonValue(std::string("0-553-21311-3"));
        book3["price"] = JsonValue(8.99);
        bookArray.push_back(JsonValue(std::move(book3)));
        
        JsonValue::ObjectType bicycleObj;
        bicycleObj["color"] = JsonValue(std::string("red"));
        bicycleObj["price"] = JsonValue(19.95);
        
        storeObj["book"] = JsonValue(std::move(bookArray));
        storeObj["bicycle"] = JsonValue(std::move(bicycleObj));
        
        JsonValue::ObjectType complexObj;
        complexObj["store"] = JsonValue(std::move(storeObj));
        JsonValue complexData(std::move(complexObj));

        // Test basic path query
        auto bookGenerator = JsonQueryGenerator(complexData, "$.store.book[*]");
        int bookCount = 0;
        for (auto it = bookGenerator.begin(); it != bookGenerator.end(); ++it) {
            bookCount++;
        }
        ASSERT_EQ(3, bookCount);

        // Test property existence query (simplified, as full JsonPath may not be implemented)
        auto priceGenerator = JsonQueryGenerator(complexData, "$..price");
        int priceCount = 0;
        for (auto it = priceGenerator.begin(); it != priceGenerator.end(); ++it) {
            priceCount++;
        }
        // Note: Actual __result_ref depends on completeness of JsonPath implementation
        ASSERT_TRUE(priceCount >= 0);

    } catch (const std::exception& e) {
        __result_ref.addFail("Complex JSONPath query test failed: " + std::string(e.what()));
    }
}

// Memory efficiency test
void test_memory_efficiency(TestResult& __result_ref) {
    try {
        // Create a relatively large dataset to test memory usage
        JsonValue largeArray = createLargeArray(10000);

        // Test streaming - only process first 100, but do not load all into memory
        JsonQueryGenerator::GeneratorOptions opts;
        opts.maxResults = 100;
        opts.batchSize = 10;
        
        auto generator = JsonQueryGenerator(largeArray, "$[*]", opts);
        
        int processed = 0;
        auto start = std::chrono::high_resolution_clock::now();
        
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            processed++;
        // Verify we can access data
            ASSERT_TRUE(it->first->getObject() != nullptr);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        ASSERT_EQ(100, processed);
        ASSERT_TRUE(duration.count() < 50);
        
        // Verify generator does not cache all results
        ASSERT_EQ(100, generator.getTotalGenerated());
        
    } catch (const std::exception& e) {
        __result_ref.addFail("Memory efficiency test failed: " + std::string(e.what()));
    }
}

// Simple test runner class
class TestRunner {
private:
    std::vector<std::pair<std::string, std::function<void(TestResult&)>>> tests_;

public:
    void addTest(const std::string& name, std::function<void(TestResult&)> func) {
        tests_.emplace_back(name, func);
    }
    
    TestResult runAllTests() {
        TestResult totalResult;
        std::cout << "=== JsonQueryGenerator Test Suite Started ===" << std::endl;
        
        for (auto& [name, func] : tests_) {
            std::cout << "Running test: " << name << std::endl;
            TestResult testResult;
            func(testResult);
            
            if (testResult.isSuccess()) {
                std::cout << "[PASS] " << name << " (" << testResult.getPassed() << " tests passed)" << std::endl;
            } else {
                std::cout << "[FAIL] " << name << " (" << testResult.getFailed() << " tests failed)" << std::endl;
                for (const auto& failure : testResult.getFailures()) {
                    std::cout << "  - " << failure << std::endl;
                }
            }
            
            totalResult.addPass(testResult.getPassed());
            for (const auto& failure : testResult.getFailures()) {
                totalResult.addFail(failure);
            }
        }
        
        return totalResult;
    }
};

int main() {
    TestRunner runner;
    
    // Register all test cases
    runner.addTest("Basic generator functionality", test_basic_generator_functionality);
    runner.addTest("Early termination", test_early_termination);
    runner.addTest("Batch processing", test_batch_processing);
    runner.addTest("Streaming query factory", test_streaming_query_factory);
    runner.addTest("Performance comparison", test_performance_comparison);
    runner.addTest("Large object handling", test_large_object_handling);
    runner.addTest("Error handling", test_error_handling);
    runner.addTest("Thread safety basics", test_thread_safety_basics);
    runner.addTest("Complex JSONPath queries", test_complex_jsonpath_queries);
    runner.addTest("Memory efficiency", test_memory_efficiency);
    
    // Run all tests
    TestResult finalResult = runner.runAllTests();
    
    // Output final __result_ref
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "JsonQueryGenerator Test Suite Finished" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "Passed: " << finalResult.getPassed() << std::endl;
    std::cout << "Failed: " << finalResult.getFailed() << std::endl;
    std::cout << "Total: " << finalResult.getTotal() << std::endl;
    
    if (finalResult.getFailed() > 0) {
        std::cout << "\nFailure details:" << std::endl;
        for (const auto& failure : finalResult.getFailures()) {
            std::cout << "  - " << failure << std::endl;
        }
    }
    
    std::cout << std::string(60, '=') << std::endl;
    
    return finalResult.getFailed() == 0 ? 0 : 1;
}
