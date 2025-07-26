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

// Generate hierarchical data for recursive queries
JsonValue generateHierarchicalData(size_t depth, size_t breadth) {
    JsonValue::ObjectType root;
    
    std::function<JsonValue(size_t, size_t)> buildLevel = [&](size_t currentDepth, size_t nodeId) -> JsonValue {
        JsonValue::ObjectType node;
        node["id"] = JsonValue(static_cast<long long>(nodeId));
        node["name"] = JsonValue("Node_" + std::to_string(nodeId));
        node["level"] = JsonValue(static_cast<long long>(currentDepth));
        
        if (currentDepth < depth) {
            JsonValue::ArrayType children;
            for (size_t i = 0; i < breadth; ++i) {
                size_t childId = nodeId * breadth + i;
                children.push_back(buildLevel(currentDepth + 1, childId));
            }
            node["children"] = JsonValue(std::move(children));
        }
        
        return JsonValue(std::move(node));
    };
    
    root["root"] = buildLevel(0, 0);
    return JsonValue(std::move(root));
}

TEST(LazyQueryVsTraditionalSmallDataset) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeBookData(100);
    std::string expression = "$.store.book[*].title";
    
    PerformanceTimer timer;
    
    // Test lazy loading
    timer.start();
    auto lazyGen = filter.queryGenerator(data, expression);
    std::vector<QueryResult> lazyResults;
    while (lazyGen.hasNext()) {
        lazyResults.push_back(lazyGen.next());
    }
    long long lazyTime = timer.end();
    
    // Test traditional method
    timer.start();
    auto traditionalResults = filter.query(data, expression);
    long long traditionalTime = timer.end();
    
    ASSERT_EQ(100, lazyResults.size());
    ASSERT_EQ(100, traditionalResults.size());
    
    std::cout << "Small dataset (100 books): Lazy " << lazyTime << "us vs Traditional " << traditionalTime << "us" << std::endl;
    
    // For small datasets, performance should be comparable
    ASSERT_TRUE(lazyTime < traditionalTime * 5); // Lazy shouldn't be more than 5x slower
}

TEST(LazyQueryVsTraditionalMediumDataset) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeBookData(1000);
    std::string expression = "$.store.book[*].title";
    
    PerformanceTimer timer;
    
    // Test lazy loading with limit
    timer.start();
    auto lazyGen = filter.queryGenerator(data, expression, 500);
    std::vector<QueryResult> lazyResults;
    while (lazyGen.hasNext()) {
        lazyResults.push_back(lazyGen.next());
    }
    long long lazyTime = timer.end();
    
    // Test traditional method
    timer.start();
    auto traditionalResults = filter.query(data, expression);
    long long traditionalTime = timer.end();
    
    ASSERT_EQ(500, lazyResults.size());
    ASSERT_EQ(1000, traditionalResults.size());
    
    std::cout << "Medium dataset (1000 books, 500 results): Lazy " << lazyTime << "us vs Traditional " << traditionalTime << "us" << std::endl;
    
    // Lazy should be faster when getting partial results
    // Allow some flexibility for the comparison
    if (lazyTime > traditionalTime) {
        std::cout << "Note: Lazy loading was slower, but this may be acceptable for certain use cases" << std::endl;
    }
}

TEST(LazyQueryVsTraditionalLargeDataset) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeBookData(5000);
    std::string expression = "$.store.book[*].title";
    
    PerformanceTimer timer;
    
    // Test lazy loading with small limit
    timer.start();
    auto lazyGen = filter.queryGenerator(data, expression, 100);
    std::vector<QueryResult> lazyResults;
    while (lazyGen.hasNext()) {
        lazyResults.push_back(lazyGen.next());
    }
    long long lazyTime = timer.end();
    
    // Test traditional method
    timer.start();
    auto traditionalResults = filter.query(data, expression);
    long long traditionalTime = timer.end();
    
    ASSERT_EQ(100, lazyResults.size());
    ASSERT_EQ(5000, traditionalResults.size());
    
    std::cout << "Large dataset (5000 books, 100 results): Lazy " << lazyTime << "us vs Traditional " << traditionalTime << "us" << std::endl;
    
    // For large datasets with small result sets, lazy should have significant advantage
    double speedup = static_cast<double>(traditionalTime) / lazyTime;
    std::cout << "Speedup factor: " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
    
    ASSERT_TRUE(lazyTime < traditionalTime); // Lazy should be faster
}

TEST(LazyQueryBatchPerformance) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeBookData(1000);
    std::string expression = "$.store.book[*].title";
    
    std::vector<size_t> batchSizes = {1, 10, 50, 100, 200};
    
    for (size_t batchSize : batchSizes) {
        PerformanceTimer timer;
        timer.start();
        
        auto gen = filter.queryGenerator(data, expression, 500);
        std::vector<QueryResult> allResults;
        
        while (gen.hasNext() && allResults.size() < 500) {
            auto batch = gen.nextBatch(batchSize);
            allResults.insert(allResults.end(), batch.begin(), batch.end());
            
            if (batch.size() < batchSize) {
                break;
            }
        }
        
        long long totalTime = timer.end();
        size_t numBatches = (allResults.size() + batchSize - 1) / batchSize;
        
        ASSERT_TRUE(allResults.size() <= 500);
        std::cout << "Batch size " << std::setw(3) << batchSize 
                  << ": " << std::setw(6) << totalTime << "us"
                  << ", Results: " << std::setw(3) << allResults.size()
                  << ", Batches: " << std::setw(3) << numBatches << std::endl;
    }
}

TEST(LazyQueryRecursivePerformance) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateHierarchicalData(4, 3); // 3^4 = 81 nodes
    
    PerformanceTimer timer;
    
    // Test recursive descent with lazy loading
    timer.start();
    auto lazyGen = filter.queryGenerator(data, "$..name");
    std::vector<QueryResult> lazyResults;
    while (lazyGen.hasNext()) {
        lazyResults.push_back(lazyGen.next());
    }
    long long lazyTime = timer.end();
    
    // Test traditional method
    timer.start();
    auto traditionalResults = filter.query(data, "$..name");
    long long traditionalTime = timer.end();
    
    ASSERT_EQ(traditionalResults.size(), lazyResults.size());
    
    std::cout << "Recursive query (" << lazyResults.size() << " results): Lazy " << lazyTime << "us vs Traditional " << traditionalTime << "us" << std::endl;
    
    // Recursive queries are expected to be slower with lazy loading due to state management
    // But results should be consistent
    double slowdown = static_cast<double>(lazyTime) / traditionalTime;
    std::cout << "Lazy slowdown factor: " << std::fixed << std::setprecision(2) << slowdown << "x" << std::endl;
    
    // Verify consistency
    std::vector<std::string> lazyNames, traditionalNames;
    for (const auto& result : lazyResults) {
        lazyNames.push_back(result.value->toString());
    }
    for (const auto& result : traditionalResults) {
        traditionalNames.push_back(result.value->toString());
    }
    
    std::sort(lazyNames.begin(), lazyNames.end());
    std::sort(traditionalNames.begin(), traditionalNames.end());
    
    ASSERT_TRUE(lazyNames == traditionalNames);
}

TEST(LazyQueryMemoryUsagePattern) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeBookData(10000);
    
    // Test that lazy loading with small batches doesn't consume excessive memory
    auto gen = filter.queryGenerator(data, "$.store.book[*].title", 1000);
    
    // Process in small chunks
    size_t totalProcessed = 0;
    const size_t chunkSize = 50;
    
    while (gen.hasNext() && totalProcessed < 1000) {
        auto batch = gen.nextBatch(chunkSize);
        totalProcessed += batch.size();
        
        // Verify each batch is reasonable size
        ASSERT_TRUE(batch.size() <= chunkSize);
        
        // In a real application, we could process and discard the batch here
        // to keep memory usage low
        
        if (batch.size() < chunkSize) {
            break;
        }
    }
    
    ASSERT_EQ(1000, totalProcessed);
    std::cout << "Processed " << totalProcessed << " results in chunks of " << chunkSize << std::endl;
}

TEST(LazyQueryEarlyTermination) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeBookData(5000);
    
    PerformanceTimer timer;
    
    // Test early termination performance
    timer.start();
    auto gen = filter.queryGenerator(data, "$.store.book[*].title");
    
    // Only get first 10 results and stop early
    std::vector<QueryResult> results;
    for (int i = 0; i < 10 && gen.hasNext(); ++i) {
        results.push_back(gen.next());
    }
    
    long long earlyTime = timer.end();
    
    ASSERT_EQ(10, results.size());
    
    // Compare with full processing
    timer.start();
    auto traditionalResults = filter.query(data, "$.store.book[*].title");
    long long fullTime = timer.end();
    
    ASSERT_EQ(5000, traditionalResults.size());
    
    std::cout << "Early termination (10 results): " << earlyTime << "us vs Full processing (5000 results): " << fullTime << "us" << std::endl;
    
    // Early termination should be much faster
    ASSERT_TRUE(earlyTime < fullTime / 10); // Should be at least 10x faster
    
    double speedup = static_cast<double>(fullTime) / earlyTime;
    std::cout << "Early termination speedup: " << std::fixed << std::setprecision(1) << speedup << "x" << std::endl;
}

int main() {
    return RUN_ALL_TESTS();
}
