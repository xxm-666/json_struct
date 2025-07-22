#include "json_engine/json_query_generator.h"
#include "json_engine/json_filter.h"
#include "json_engine/json_value.h"
#include <chrono>
#include <iostream>

using namespace JsonStruct;

int main() {
    // Create a large test dataset
    JsonValue::ArrayType largeArray;
    for (int i = 0; i < 10000; ++i) {
        JsonValue::ObjectType obj;
        obj["id"] = JsonValue(i);
        obj["name"] = JsonValue(std::string("item_" + std::to_string(i)));
        obj["value"] = JsonValue(i * 10);
        largeArray.push_back(JsonValue(std::move(obj)));
    }
    JsonValue largeData(std::move(largeArray));
    
    std::cout << "=== Performance Comparison Test ===" << std::endl;
    std::cout << "Dataset size: 10,000 objects" << std::endl;
    
    // Test 1: Use traditional JsonFilter.query() to query all results at once
    std::cout << "\n1. Traditional method (query all results at once):" << std::endl;
    auto start1 = std::chrono::high_resolution_clock::now();
    
    JsonFilter filter = JsonFilter::createDefault();
    auto allResults = filter.query(largeData, "$[*].name");
    
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);
    
    std::cout << "Query time: " << duration1.count() << " microseconds" << std::endl;
    std::cout << "Number of results: " << allResults.size() << std::endl;
    
    // Test 2: Use lazy loading to get only the first 10 results
    std::cout << "\n2. Lazy loading method (retrieve only the first 10 results):" << std::endl;
    auto start2 = std::chrono::high_resolution_clock::now();
    
    JsonQueryGenerator::GeneratorOptions options;
    options.maxResults = 10;
    auto generator = JsonQueryGenerator(largeData, "$[*].name", options);
    
    int count = 0;
    for (auto it = generator.begin(); it != generator.end(); ++it) {
        count++;
        if (count >= 10) break;
    }
    
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);
    
    std::cout << "Query time: " << duration2.count() << " microseconds" << std::endl;
    std::cout << "Number of results: " << count << std::endl;
    
    // Test 3: Lazy loading but retrieve all results (compare with traditional method)
    std::cout << "\n3. Lazy loading method (retrieve all results):" << std::endl;
    auto start3 = std::chrono::high_resolution_clock::now();
    
    auto generator2 = JsonQueryGenerator(largeData, "$[*].name");
    count = 0;
    for (auto it = generator2.begin(); it != generator2.end(); ++it) {
        count++;
    }
    
    auto end3 = std::chrono::high_resolution_clock::now();
    auto duration3 = std::chrono::duration_cast<std::chrono::microseconds>(end3 - start3);
    
    std::cout << "Query time: " << duration3.count() << " microseconds" << std::endl;
    std::cout << "Number of results: " << count << std::endl;
    
    // Performance improvement calculation
    std::cout << "\n=== Performance Analysis ===" << std::endl;
    if (duration1.count() > 0) {
        double improvement = (double)(duration1.count() - duration2.count()) / duration1.count() * 100;
        std::cout << "Performance improvement of lazy loading (first 10) compared to full query: " << improvement << "%" << std::endl;
    }
    
    std::cout << "\n=== Conclusion ===" << std::endl;
    std::cout << "Lazy loading should be significantly faster when only partial results are needed" << std::endl;
    std::cout << "Lazy loading may be comparable to the traditional method when all results are needed" << std::endl;
    
    return 0;
}
