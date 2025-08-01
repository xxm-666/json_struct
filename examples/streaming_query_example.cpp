#include "../src/json_engine/json_value.h"
#include "../src/json_engine/json_query_generator.h"
#include <iostream>
#include <chrono>
#include <memory>

using namespace JsonStruct;

int main() {
    // Create a large JSON structure for demonstration
    JsonValue root = JsonValue::object();
    
    // Add a large array to test streaming performance
    JsonValue largeArray = JsonValue::array({});
    for (int i = 0; i < 10000; ++i) {
        JsonValue item = JsonValue::object({
            {"id", JsonValue(i)},
            {"name", JsonValue("item_" + std::to_string(i))},
            {"value", JsonValue(i * 1.5)},
            {"active", JsonValue(i % 2 == 0)}
        });
        largeArray.append(std::move(item));
    }
    
    root["data"] = std::move(largeArray);
    root["metadata"] = JsonValue::object({
        {"count", JsonValue(10000)},
        {"version", JsonValue("1.0")},
        {"generated", JsonValue("2025-07-14")}
    });
    
    std::cout << "=== Streaming Query Demo ===" << std::endl;
    std::cout << "JSON structure created with 10,000 items" << std::endl << std::endl;
    
    // 1. Traditional query (loads all results at once)
    std::cout << "1. Traditional Query (selectAll):" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    
    auto allResults = root.selectAll("$.data[*].name");
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "   Found " << allResults.size() << " results in " << duration.count() << "ms" << std::endl;
    std::cout << "   Memory usage: ~" << (allResults.size() * sizeof(JsonValue*)) << " bytes for pointers" << std::endl;
    std::cout << std::endl;
    
    // 2. Streaming query with early termination
    std::cout << "2. Streaming Query with Early Termination (first 5 results):" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    
    JsonQueryGenerator::GeneratorOptions options;
    options.maxResults = 5;
    
    auto generator = JsonStreamingQuery::createGenerator(root, "$.data[*].name", options);
    
    std::cout << "   Results:" << std::endl;
    for (auto it = generator.begin(); it != generator.end(); ++it) {
        if (auto str = it->first->getString()) {
            std::cout << "     [" << it.getIndex() << "] " << *str << " (path: " << it->second << ")" << std::endl;
        }
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "   Generated " << generator.getTotalGenerated() << " results in " << duration.count() << "ms" << std::endl;
    std::cout << std::endl;
    
    // 3. Lazy processing with custom function
    std::cout << "3. Lazy Processing with Custom Function:" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    
    size_t processedCount = JsonStreamingQuery::lazyQuery(root, "$.data[*]", 
        [](const JsonValue* value, const std::string& path) {
            // Process only items with even ID
            if (auto idValue = (*value)["id"].getInteger()) {
                if (*idValue % 2 == 0 && *idValue < 20) {
                    std::cout << "     Processing item with ID: " << *idValue << std::endl;
                    return true;  // Continue processing
                }
                if (*idValue >= 20) {
                    return false; // Stop processing
                }
            }
            return true;
        });
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "   Processed " << processedCount << " items in " << duration.count() << "ms" << std::endl;
    std::cout << std::endl;
    
    // 4. Count matches without materializing results
    std::cout << "4. Count Matches (no materialization):" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    
    size_t activeCount = JsonStreamingQuery::countMatches(root, "$.data[?(@.active == true)]");
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "   Found " << activeCount << " active items in " << duration.count() << "ms" << std::endl;
    std::cout << std::endl;
    
    // 5. Find first match efficiently
    std::cout << "5. Find First Match (early termination):" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    
    auto firstMatch = JsonStreamingQuery::findFirst(root, "$.data[?(@.id > 5000)]");
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    if (firstMatch) {
        auto idValue = (*firstMatch->first)["id"].getInteger();
        std::cout << "   First match found: ID = " << (idValue ? *idValue : -1) 
                  << " in " << duration.count() << "ms" << std::endl;
        std::cout << "   Path: " << firstMatch->second << std::endl;
    } else {
        std::cout << "   No match found in " << duration.count() << "ms" << std::endl;
    }
    std::cout << std::endl;
    
    // 6. Batch processing
    std::cout << "6. Batch Processing (batches of 100):" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    
    JsonQueryGenerator::GeneratorOptions batchOptions;
    batchOptions.batchSize = 100;
    batchOptions.maxResults = 500;  // Process only first 500
    
    auto batchGenerator = JsonStreamingQuery::createGenerator(root, "$.data[*].value", batchOptions);
    
    size_t batchCount = 0;
    size_t totalProcessed = 0;
    
    while (batchGenerator.hasMore()) {
        auto batch = batchGenerator.takeBatch(100);
        if (batch.empty()) break;
        
        ++batchCount;
        totalProcessed += batch.size();
        
        // Process batch
        double batchSum = 0.0;
        for (const auto& [value, path] : batch) {
            if (auto num = value->getNumber()) {
                batchSum += *num;
            }
        }
        
        std::cout << "     Batch " << batchCount << ": " << batch.size() 
                  << " items, sum = " << batchSum << std::endl;
        
        if (batchCount >= 5) break; // Limit output
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "   Processed " << totalProcessed << " items in " << batchCount 
              << " batches in " << duration.count() << "ms" << std::endl;
    std::cout << std::endl;
    
    // 7. Custom yield processing
    std::cout << "7. Custom Yield Processing:" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    
    size_t yieldCount = 0;
    double sum = 0.0;
    
    generator = JsonStreamingQuery::createGenerator(root, "$.data[*].value");
    generator.yield([&](const JsonValue* value, const std::string& path, size_t index) -> bool {
        if (auto num = value->getNumber()) {
            sum += *num;
            ++yieldCount;
            
            // Stop after first 1000 items
            if (yieldCount >= 1000) {
                return false;
            }
        }
        return true;
    });
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "   Processed " << yieldCount << " values, sum = " << sum 
              << " in " << duration.count() << "ms" << std::endl;
    std::cout << std::endl;
    
    std::cout << "=== Demo Complete ===" << std::endl;
    std::cout << "Streaming queries provide:" << std::endl;
    std::cout << "- Memory efficiency for large datasets" << std::endl;
    std::cout << "- Early termination for better performance" << std::endl;
    std::cout << "- Batch processing capabilities" << std::endl;
    std::cout << "- Custom processing with generators" << std::endl;
    
    return 0;
}
