#include "../test_framework/test_framework.h"
#include "../src/json_engine/json_filter.h"
#include "../src/json_engine/json_value.h"
#include "../src/json_engine/lazy_query_generator.h"
#include <chrono>
#include <iomanip>
#include <vector>
#include <string>

using namespace JsonStruct;
using namespace TestFramework;

// Helper class for performance measurement
class PerformanceTimer {
public:
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }
    
    long long end() {
        auto end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    }
    
private:
    std::chrono::high_resolution_clock::time_point start_time;
};

// Generate large test dataset
JsonValue generateLargeDataset(size_t size) {
    JsonValue::ArrayType largeArray;
    for (size_t i = 0; i < size; ++i) {
        JsonValue::ObjectType obj;
        obj["id"] = JsonValue(static_cast<long long>(i));
        obj["name"] = JsonValue("item_" + std::to_string(i));
        obj["value"] = JsonValue(static_cast<long long>(i * 10));
        obj["category"] = JsonValue(i % 5 == 0 ? "special" : "normal");
        largeArray.push_back(JsonValue(std::move(obj)));
    }
    return JsonValue(std::move(largeArray));
}

TEST(TraditionalQueryAllResults) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeDataset(10000);
    
    PerformanceTimer timer;
    timer.start();
    auto allResults = filter.query(data, "$[*].name");
    long long duration = timer.end();
    
    ASSERT_EQ(10000, allResults.size());
    std::cout << "Traditional query all results: " << duration << " microseconds" << std::endl;
    
    // Verify first few results
    ASSERT_EQ("item_0", allResults[0].value->toString());
    ASSERT_EQ("item_1", allResults[1].value->toString());
    ASSERT_EQ("item_9999", allResults[9999].value->toString());
}

TEST(LazyQueryFirst10Results) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeDataset(10000);
    
    PerformanceTimer timer;
    timer.start();
    auto generator = filter.queryGenerator(data, "$[*].name", 10);
    
    std::vector<QueryResult> lazyResults;
    while (generator.hasNext()) {
        lazyResults.push_back(generator.next());
    }
    
    long long duration = timer.end();
    
    ASSERT_EQ(10, lazyResults.size());
    std::cout << "Lazy query first 10 results: " << duration << " microseconds" << std::endl;
    
    // Verify results
    ASSERT_EQ("item_0", lazyResults[0].value->toString());
    ASSERT_EQ("item_9", lazyResults[9].value->toString());
}

TEST(LazyQueryAllResults) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeDataset(10000);
    
    PerformanceTimer timer;
    timer.start();
    auto generator = filter.queryGenerator(data, "$[*].name");
    
    std::vector<QueryResult> lazyResults;
    while (generator.hasNext()) {
        lazyResults.push_back(generator.next());
    }
    
    long long duration = timer.end();
    
    ASSERT_EQ(10000, lazyResults.size());
    std::cout << "Lazy query all results: " << duration << " microseconds" << std::endl;
    
    // Verify results
    ASSERT_EQ("item_0", lazyResults[0].value->toString());
    ASSERT_EQ("item_9999", lazyResults[9999].value->toString());
}

TEST(PerformanceComparisonAnalysis) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeDataset(10000);
    
    PerformanceTimer timer;
    
    // Traditional method
    timer.start();
    auto traditionalResults = filter.query(data, "$[*].name");
    long long traditionalTime = timer.end();
    
    // Lazy method - first 10
    timer.start();
    auto lazyGen = filter.queryGenerator(data, "$[*].name", 10);
    std::vector<QueryResult> lazyPartial;
    while (lazyGen.hasNext()) {
        lazyPartial.push_back(lazyGen.next());
    }
    long long lazyPartialTime = timer.end();
    
    // Lazy method - all results
    timer.start();
    auto lazyGenAll = filter.queryGenerator(data, "$[*].name");
    std::vector<QueryResult> lazyAll;
    while (lazyGenAll.hasNext()) {
        lazyAll.push_back(lazyGenAll.next());
    }
    long long lazyAllTime = timer.end();
    
    // Assertions
    ASSERT_EQ(10000, traditionalResults.size());
    ASSERT_EQ(10, lazyPartial.size());
    ASSERT_EQ(10000, lazyAll.size());
    
    // Performance analysis
    std::cout << "=== Performance Analysis ===" << std::endl;
    std::cout << "Traditional (all): " << traditionalTime << " μs" << std::endl;
    std::cout << "Lazy (first 10):   " << lazyPartialTime << " μs" << std::endl;
    std::cout << "Lazy (all):        " << lazyAllTime << " μs" << std::endl;
    
    if (traditionalTime > 0) {
        double partialImprovement = static_cast<double>(traditionalTime - lazyPartialTime) / traditionalTime * 100;
        std::cout << "Lazy partial improvement: " << std::fixed << std::setprecision(1) 
                  << partialImprovement << "%" << std::endl;
    }
    
    // Lazy partial should be faster for partial results
    ASSERT_TRUE(lazyPartialTime < traditionalTime);
    
    // Verify result consistency
    for (size_t i = 0; i < 10; ++i) {
        ASSERT_EQ(traditionalResults[i].value->toString(), lazyPartial[i].value->toString());
        ASSERT_EQ(traditionalResults[i].value->toString(), lazyAll[i].value->toString());
    }
}

TEST(LazyQueryProgressiveRetrieval) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeDataset(1000);
    
    std::vector<size_t> batchSizes = {1, 5, 10, 50, 100};
    
    for (size_t batchSize : batchSizes) {
        PerformanceTimer timer;
        timer.start();
        
        auto generator = filter.queryGenerator(data, "$[*].name", 100);
        std::vector<QueryResult> results;
        
        while (generator.hasNext() && results.size() < 100) {
            auto batch = generator.nextBatch(batchSize);
            results.insert(results.end(), batch.begin(), batch.end());
        }
        
        long long duration = timer.end();
        
        ASSERT_TRUE(results.size() <= 100);
        std::cout << "Batch size " << std::setw(3) << batchSize 
                  << ": " << std::setw(6) << duration << " μs" 
                  << ", Results: " << results.size() << std::endl;
    }
}

TEST(LazyQueryMemoryEfficiency) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeDataset(5000);
    
    // Simulate processing large dataset in chunks
    const size_t chunkSize = 100;
    size_t totalProcessed = 0;
    
    auto generator = filter.queryGenerator(data, "$[*].name");
    
    PerformanceTimer timer;
    timer.start();
    
    while (generator.hasNext() && totalProcessed < 1000) {
        auto batch = generator.nextBatch(chunkSize);
        
        // Simulate processing each chunk
        for (const auto& item : batch) {
            ASSERT_TRUE(item.value->toString().find("item_") == 0);
        }
        
        totalProcessed += batch.size();
        
        // In real scenario, batch would be processed and discarded
        if (batch.size() < chunkSize) {
            break;
        }
    }
    
    long long duration = timer.end();
    
    ASSERT_EQ(1000, totalProcessed);
    std::cout << "Memory efficient processing: " << duration << " μs for " 
              << totalProcessed << " items in chunks of " << chunkSize << std::endl;
}

int main() {
    std::cout << "=== Legacy Performance Test Suite ===" << std::endl;
    std::cout << "Dataset size: 10,000 objects" << std::endl;
    std::cout << "Testing traditional vs lazy query performance" << std::endl << std::endl;
    
    return RUN_ALL_TESTS();
}
