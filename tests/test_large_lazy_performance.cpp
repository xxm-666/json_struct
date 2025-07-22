#include <iostream>
#include <chrono>
#include <vector>
#include "json_engine/json_filter.h"
#include "json_engine/json_value.h"

using namespace JsonStruct;

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
    JsonObject store;
    JsonArray books;
    
    for (size_t i = 0; i < bookCount; ++i) {
        JsonObject book;
        book["title"] = JsonValue("Book " + std::to_string(i + 1));
        book["author"] = JsonValue("Author " + std::to_string(i % 20 + 1)); // 20 different authors
        book["price"] = JsonValue(10.0 + (i % 50)); // Prices from 10 to 59
        book["category"] = JsonValue(i % 3 == 0 ? "fiction" : (i % 3 == 1 ? "science" : "history"));
        book["isbn"] = JsonValue("ISBN-" + std::to_string(1000000 + i));
        books.push_back(JsonValue(std::move(book)));
    }
    
    store["book"] = JsonValue(std::move(books));
    
    JsonObject root;
    root["store"] = JsonValue(std::move(store));
    
    return JsonValue(std::move(root));
}

void testLazyPerformanceAtDifferentSizes() {
    JsonFilter filter = JsonFilter::createDefault();
    
    std::vector<size_t> testSizes = {25, 50, 75, 100, 200, 500, 1000};
    std::string expression = "$.store.book[*].title";
    
    std::cout << "Testing lazy loading performance at different result sizes:\n";
    std::cout << "Expected Results | Data Size | Lazy Time (μs) | Traditional Time (μs) | Fast Path Time (μs)\n";
    std::cout << "-----------------|-----------|---------------|---------------------|------------------\n";
    
    for (size_t expectedResults : testSizes) {
        // Generate data with enough books to get the expected results
        size_t dataSize = std::max(expectedResults, static_cast<size_t>(100));
        JsonValue data = generateLargeBookData(dataSize);
        
        PerformanceTimer timer;
        
        // Test optimized lazy loading (our current implementation)
        timer.start();
        auto lazyGen = filter.queryGeneratorOptimized(data, expression, expectedResults);
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
        
        // Test fast path (for comparison)
        timer.start();
        auto fastResults = filter.queryFast(data, expression, expectedResults);
        long long fastTime = timer.end();
        
        std::cout << std::setw(16) << expectedResults
                  << " | " << std::setw(9) << dataSize
                  << " | " << std::setw(13) << lazyTime
                  << " | " << std::setw(19) << traditionalTime
                  << " | " << std::setw(17) << fastTime << "\n";
        
        // Verify correctness
        if (lazyResults.size() != std::min(expectedResults, fastResults.size())) {
            std::cout << "ERROR: Result count mismatch for " << expectedResults << " results!\n";
            std::cout << "Lazy: " << lazyResults.size() << ", Fast: " << fastResults.size() << "\n";
        }
    }
}

void testBatchedLazyApproach() {
    std::cout << "\n\nTesting batched lazy approach for large result sets:\n";
    
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeBookData(1000);
    std::string expression = "$.store.book[*].title";
    
    // Test different batch sizes for 200 results
    std::vector<size_t> batchSizes = {10, 25, 50, 100};
    
    std::cout << "Batch Size | Total Time (μs) | Avg per batch (μs)\n";
    std::cout << "-----------|----------------|------------------\n";
    
    for (size_t batchSize : batchSizes) {
        PerformanceTimer timer;
        timer.start();
        
        auto lazyGen = filter.queryGeneratorOptimized(data, expression, 200);
        std::vector<QueryResult> allResults;
        
        while (lazyGen.hasNext() && allResults.size() < 200) {
            auto batch = lazyGen.nextBatch(batchSize);
            allResults.insert(allResults.end(), batch.begin(), batch.end());
            
            if (batch.size() < batchSize) {
                break; // No more results
            }
        }
        
        long long totalTime = timer.end();
        size_t numBatches = (allResults.size() + batchSize - 1) / batchSize;
        
        std::cout << std::setw(10) << batchSize
                  << " | " << std::setw(14) << totalTime
                  << " | " << std::setw(17) << (numBatches > 0 ? totalTime / numBatches : 0) << "\n";
        
        std::cout << "  Results: " << allResults.size() << "\n";
    }
}

int main() {
    try {
        testLazyPerformanceAtDifferentSizes();
        testBatchedLazyApproach();
        
        std::cout << "\n\nAnalysis:\n";
        std::cout << "- For results <= 50: Fast path is used (excellent performance)\n";
        std::cout << "- For results > 50: Traditional lazy loading is used (poor performance)\n";
        std::cout << "- Need to implement better strategy for large result sets\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
