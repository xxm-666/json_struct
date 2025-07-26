#include "../test_framework/test_framework.h"
#include "../src/json_engine/json_filter.h"
#include "../src/json_engine/json_value.h"
#include "../src/json_engine/lazy_query_generator.h"
#include <chrono>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>

using namespace JsonStruct;
using namespace TestFramework;

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

// Generate test data with many books
JsonValue generateLargeBookData(size_t bookCount) {
    JsonValue::ObjectType store;
    JsonValue::ArrayType books;
    
    for (size_t i = 0; i < bookCount; ++i) {
        JsonValue::ObjectType book;
        book["title"] = JsonValue("Book " + std::to_string(i + 1));
        book["author"] = JsonValue("Author " + std::to_string(i % 20 + 1)); // 20 different authors
        book["price"] = JsonValue(10.0 + (i % 50)); // Prices from 10 to 59
        book["category"] = JsonValue(i % 3 == 0 ? "fiction" : (i % 3 == 1 ? "science" : "history"));
        book["isbn"] = JsonValue("ISBN-" + std::to_string(1000000 + i));
        books.push_back(JsonValue(std::move(book)));
    }
    
    store["book"] = JsonValue(std::move(books));
    
    JsonValue::ObjectType root;
    root["store"] = JsonValue(std::move(store));
    
    return JsonValue(std::move(root));
}

TEST(LargeDataPerformanceComparison25Books) {
    JsonFilter filter = JsonFilter::createDefault();
    const size_t expectedResults = 25;
    const size_t dataSize = 100;
    JsonValue data = generateLargeBookData(dataSize);
    std::string expression = "$.store.book[*].title";
    
    PerformanceTimer timer;
    
    // Test lazy loading
    timer.start();
    auto lazyGen = filter.queryGenerator(data, expression, expectedResults);
    std::vector<QueryResult> lazyResults;
    while (lazyGen.hasNext() && lazyResults.size() < expectedResults) {
        lazyResults.push_back(lazyGen.next());
    }
    long long lazyTime = timer.end();
    
    // Test traditional method
    timer.start();
    auto traditionalResults = filter.query(data, expression);
    if (traditionalResults.size() > expectedResults) {
        traditionalResults.resize(expectedResults);
    }
    long long traditionalTime = timer.end();
    
    ASSERT_EQ(expectedResults, lazyResults.size());
    ASSERT_TRUE(traditionalResults.size() >= expectedResults);
    
    std::cout << "25 Books - Lazy: " << lazyTime << "μs, Traditional: " << traditionalTime << "μs" << std::endl;
    
    // Verify results correctness
    for (size_t i = 0; i < expectedResults; ++i) {
        ASSERT_EQ(traditionalResults[i].value->toString(), lazyResults[i].value->toString());
    }
}

TEST(LargeDataPerformanceComparison100Books) {
    JsonFilter filter = JsonFilter::createDefault();
    const size_t expectedResults = 100;
    const size_t dataSize = 200;
    JsonValue data = generateLargeBookData(dataSize);
    std::string expression = "$.store.book[*].title";
    
    PerformanceTimer timer;
    
    // Test lazy loading
    timer.start();
    auto lazyGen = filter.queryGenerator(data, expression, expectedResults);
    std::vector<QueryResult> lazyResults;
    while (lazyGen.hasNext() && lazyResults.size() < expectedResults) {
        lazyResults.push_back(lazyGen.next());
    }
    long long lazyTime = timer.end();
    
    // Test traditional method
    timer.start();
    auto traditionalResults = filter.query(data, expression);
    if (traditionalResults.size() > expectedResults) {
        traditionalResults.resize(expectedResults);
    }
    long long traditionalTime = timer.end();
    
    ASSERT_EQ(expectedResults, lazyResults.size());
    ASSERT_TRUE(traditionalResults.size() >= expectedResults);
    
    std::cout << "100 Books - Lazy: " << lazyTime << "μs, Traditional: " << traditionalTime << "μs" << std::endl;
    
    // Verify results correctness
    for (size_t i = 0; i < expectedResults; ++i) {
        ASSERT_EQ(traditionalResults[i].value->toString(), lazyResults[i].value->toString());
    }
}

TEST(LargeDataPerformanceComparison1000Books) {
    JsonFilter filter = JsonFilter::createDefault();
    const size_t expectedResults = 1000;
    const size_t dataSize = 1000;
    JsonValue data = generateLargeBookData(dataSize);
    std::string expression = "$.store.book[*].title";
    
    PerformanceTimer timer;
    
    // Test lazy loading
    timer.start();
    auto lazyGen = filter.queryGenerator(data, expression, expectedResults);
    std::vector<QueryResult> lazyResults;
    while (lazyGen.hasNext() && lazyResults.size() < expectedResults) {
        lazyResults.push_back(lazyGen.next());
    }
    long long lazyTime = timer.end();
    
    // Test traditional method
    timer.start();
    auto traditionalResults = filter.query(data, expression);
    long long traditionalTime = timer.end();
    
    ASSERT_EQ(expectedResults, lazyResults.size());
    ASSERT_EQ(dataSize, traditionalResults.size());
    
    std::cout << "1000 Books - Lazy: " << lazyTime << "μs, Traditional: " << traditionalTime << "μs" << std::endl;
    
    // Verify results correctness
    for (size_t i = 0; i < expectedResults; ++i) {
        ASSERT_EQ(traditionalResults[i].value->toString(), lazyResults[i].value->toString());
    }
}

TEST(LargeDataPerformanceComparison10000Books) {
    JsonFilter filter = JsonFilter::createDefault();
    const size_t expectedResults = 1000; // Only get first 1000 from 10000
    const size_t dataSize = 10000;
    JsonValue data = generateLargeBookData(dataSize);
    std::string expression = "$.store.book[*].title";
    
    PerformanceTimer timer;
    
    // Test lazy loading
    timer.start();
    auto lazyGen = filter.queryGenerator(data, expression, expectedResults);
    std::vector<QueryResult> lazyResults;
    while (lazyGen.hasNext() && lazyResults.size() < expectedResults) {
        lazyResults.push_back(lazyGen.next());
    }
    long long lazyTime = timer.end();
    
    // Test traditional method
    timer.start();
    auto traditionalResults = filter.query(data, expression);
    long long traditionalTime = timer.end();
    
    ASSERT_EQ(expectedResults, lazyResults.size());
    ASSERT_EQ(dataSize, traditionalResults.size());
    
    std::cout << "10000 Books (first 1000) - Lazy: " << lazyTime << "μs, Traditional: " << traditionalTime << "μs" << std::endl;
    
    // Lazy should be significantly faster
    double speedup = static_cast<double>(traditionalTime) / lazyTime;
    std::cout << "Speedup factor: " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
    ASSERT_TRUE(lazyTime < traditionalTime);
    
    // Verify results correctness
    for (size_t i = 0; i < expectedResults; ++i) {
        ASSERT_EQ(traditionalResults[i].value->toString(), lazyResults[i].value->toString());
    }
}

TEST(BatchedLazyApproachPerformance) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeBookData(1000);
    std::string expression = "$.store.book[*].title";
    
    std::vector<size_t> batchSizes = {10, 25, 50, 100, 200};
    std::cout << "Batched approach performance test:" << std::endl;
    
    for (size_t batchSize : batchSizes) {
        PerformanceTimer timer;
        timer.start();
        
        auto lazyGen = filter.queryGenerator(data, expression, 200);
        std::vector<QueryResult> allResults;
        
        while (lazyGen.hasNext() && allResults.size() < 200) {
            auto batch = lazyGen.nextBatch(batchSize);
            allResults.insert(allResults.end(), batch.begin(), batch.end());
            
            if (batch.size() < batchSize) {
                break;
            }
        }
        
        long long totalTime = timer.end();
        size_t numBatches = (allResults.size() + batchSize - 1) / batchSize;
        
        ASSERT_TRUE(allResults.size() <= 200);
        
        std::cout << "Batch size " << std::setw(3) << batchSize 
                  << ": " << std::setw(6) << totalTime << "μs"
                  << ", Results: " << allResults.size()
                  << ", Batches: " << numBatches << std::endl;
    }
}

TEST(VeryLargeDatasetStressTest) {
    JsonFilter filter = JsonFilter::createDefault();
    const size_t expectedResults = 100; // Only get first 100 from very large dataset
    const size_t dataSize = 50000;
    JsonValue data = generateLargeBookData(dataSize);
    std::string expression = "$.store.book[*].title";
    
    PerformanceTimer timer;
    
    // Test lazy loading with early termination
    timer.start();
    auto lazyGen = filter.queryGenerator(data, expression, expectedResults);
    std::vector<QueryResult> lazyResults;
    while (lazyGen.hasNext() && lazyResults.size() < expectedResults) {
        lazyResults.push_back(lazyGen.next());
    }
    long long lazyTime = timer.end();
    
    ASSERT_EQ(expectedResults, lazyResults.size());
    
    std::cout << "Stress test - 50000 books (first 100): " << lazyTime << "μs" << std::endl;
    
    // Should complete in reasonable time (less than 1 second)
    ASSERT_TRUE(lazyTime < 1000000); // Less than 1 second
    
    // Verify results
    ASSERT_EQ("Book 1", lazyResults[0].value->toString());
    ASSERT_EQ("Book 100", lazyResults[99].value->toString());
}

TEST(PerformanceScalabilityAnalysis) {
    JsonFilter filter = JsonFilter::createDefault();
    std::vector<size_t> testSizes = {100, 500, 1000, 5000, 10000};
    const size_t resultsWanted = 50; // Always get first 50 results
    std::string expression = "$.store.book[*].title";
    
    std::cout << "Scalability analysis (first 50 results):" << std::endl;
    std::cout << "Data Size | Lazy Time (μs) | Traditional Time (μs) | Speedup" << std::endl;
    std::cout << "----------|---------------|---------------------|--------" << std::endl;
    
    for (size_t dataSize : testSizes) {
        JsonValue data = generateLargeBookData(dataSize);
        PerformanceTimer timer;
        
        // Test lazy loading
        timer.start();
        auto lazyGen = filter.queryGenerator(data, expression, resultsWanted);
        std::vector<QueryResult> lazyResults;
        while (lazyGen.hasNext() && lazyResults.size() < resultsWanted) {
            lazyResults.push_back(lazyGen.next());
        }
        long long lazyTime = timer.end();
        
        // Test traditional method
        timer.start();
        auto traditionalResults = filter.query(data, expression);
        long long traditionalTime = timer.end();
        
        ASSERT_EQ(resultsWanted, lazyResults.size());
        ASSERT_EQ(dataSize, traditionalResults.size());
        
        double speedup = static_cast<double>(traditionalTime) / lazyTime;
        
        std::cout << std::setw(9) << dataSize
                  << " | " << std::setw(13) << lazyTime
                  << " | " << std::setw(19) << traditionalTime
                  << " | " << std::fixed << std::setprecision(2) << std::setw(6) << speedup << "x" << std::endl;
    }
}

int main() {
    std::cout << "=== Large Dataset Performance Test Suite ===" << std::endl;
    std::cout << "Testing lazy loading performance at various dataset sizes" << std::endl;
    std::cout << "Focus: Early termination benefits and scalability" << std::endl << std::endl;
    
    return RUN_ALL_TESTS();
}
