#include <test_framework/test_framework.h>
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

// Create complex nested JSON data for testing
JsonValue createComplexTestData() {
    JsonValue::ObjectType root;
    
    // 1. Deep nested structure
    JsonValue::ObjectType company;
    JsonValue::ArrayType departments;
    
    for (int d = 0; d < 3; ++d) {
        JsonValue::ObjectType dept;
        dept["name"] = JsonValue("Department " + std::to_string(d));
        dept["id"] = JsonValue(static_cast<long long>(d));
        
        JsonValue::ArrayType employees;
        for (int e = 0; e < 5; ++e) {
            JsonValue::ObjectType employee;
            employee["name"] = JsonValue("Employee " + std::to_string(d) + "-" + std::to_string(e));
            employee["id"] = JsonValue(static_cast<long long>(d * 100 + e));
            employee["salary"] = JsonValue((d + 1) * 1000.0 + e * 100.0);
            
            // Nested address info
            JsonValue::ObjectType address;
            address["street"] = JsonValue("Street " + std::to_string(e));
            address["city"] = JsonValue("City " + std::to_string(d));
            address["zipcode"] = JsonValue(10000 + d * 1000 + e);
            employee["address"] = JsonValue(std::move(address));
            
            // Skills array
            JsonValue::ArrayType skills;
            for (int s = 0; s < 3; ++s) {
                skills.push_back(JsonValue("Skill" + std::to_string(s)));
            }
            employee["skills"] = JsonValue(std::move(skills));
            
            employees.push_back(JsonValue(std::move(employee)));
        }
        dept["employees"] = JsonValue(std::move(employees));
        departments.push_back(JsonValue(std::move(dept)));
    }
    
    company["departments"] = JsonValue(std::move(departments));
    company["name"] = JsonValue("TechCorp");
    company["founded"] = JsonValue(2000LL);
    
    root["company"] = JsonValue(std::move(company));
    
    // 2. Mixed array types
    JsonValue::ArrayType mixedArray;
    mixedArray.push_back(JsonValue("string_item"));
    mixedArray.push_back(JsonValue(42LL));
    mixedArray.push_back(JsonValue(3.14));
    mixedArray.push_back(JsonValue(true));
    mixedArray.push_back(JsonValue()); // null
    
    JsonValue::ObjectType objInArray;
    objInArray["type"] = JsonValue("embedded_object");
    objInArray["value"] = JsonValue(100LL);
    mixedArray.push_back(JsonValue(std::move(objInArray)));
    
    root["mixed_array"] = JsonValue(std::move(mixedArray));
    
    // 3. Special character properties
    JsonValue::ObjectType specialKeys;
    specialKeys["key-with-dash"] = JsonValue("dash_value");
    specialKeys["key.with.dots"] = JsonValue("dots_value");
    specialKeys["key with spaces"] = JsonValue("spaces_value");
    specialKeys["key[with]brackets"] = JsonValue("brackets_value");
    specialKeys["key_123"] = JsonValue("underscore_value");
    
    root["special_keys"] = JsonValue(std::move(specialKeys));
    
    // 4. Large array for performance testing
    JsonValue::ArrayType largeArray;
    for (int i = 0; i < 1000; ++i) {
        JsonValue::ObjectType item;
        item["id"] = JsonValue(static_cast<long long>(i));
        item["name"] = JsonValue("Item_" + std::to_string(i));
        item["category"] = JsonValue("Category_" + std::to_string(i % 10));
        item["price"] = JsonValue(i * 0.99);
        largeArray.push_back(JsonValue(std::move(item)));
    }
    root["large_array"] = JsonValue(std::move(largeArray));
    
    // 5. Empty structures
    root["empty_object"] = JsonValue(JsonValue::ObjectType{});
    root["empty_array"] = JsonValue(JsonValue::ArrayType{});
    
    return JsonValue(std::move(root));
}

// Generate large book data for performance testing
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

// Basic lazy query functionality tests
TEST(LazyQueryBasicFunctionality) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexTestData();
    
    // Test simple property access
    auto gen = filter.queryGenerator(data, "$.company.name");
    ASSERT_TRUE(gen.hasNext());
    auto queryResult = gen.next();
    ASSERT_EQ("TechCorp", queryResult.value->toString());
    ASSERT_FALSE(gen.hasNext());
}

TEST(LazyQueryArrayAccess) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexTestData();
    
    // Test array access
    auto gen = filter.queryGenerator(data, "$.company.departments[0].name");
    ASSERT_TRUE(gen.hasNext());
    auto queryResult = gen.next();
    ASSERT_EQ("Department 0", queryResult.value->toString());
    ASSERT_FALSE(gen.hasNext());
}

TEST(LazyQueryWildcardAccess) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexTestData();
    
    // Test wildcard access
    auto gen = filter.queryGenerator(data, "$.company.departments[*].name");
    std::vector<std::string> names;
    while (gen.hasNext()) {
        names.push_back(gen.next().value->toString());
    }
    
    ASSERT_EQ(3, names.size());
    ASSERT_EQ("Department 0", names[0]);
    ASSERT_EQ("Department 1", names[1]);
    ASSERT_EQ("Department 2", names[2]);
}

TEST(LazyQueryRecursiveDescent) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexTestData();
    
    // Test recursive descent
    auto gen = filter.queryGenerator(data, "$..name");
    std::vector<std::string> names;
    while (gen.hasNext()) {
        names.push_back(gen.next().value->toString());
    }
    
    // Should find company name + department names + employee names + item names
    // 1 + 3 + 15 + 1000 = 1019
    ASSERT_EQ(1019, names.size());
    
    // Verify some expected values
    ASSERT_TRUE(std::find(names.begin(), names.end(), "TechCorp") != names.end());
    ASSERT_TRUE(std::find(names.begin(), names.end(), "Department 0") != names.end());
    ASSERT_TRUE(std::find(names.begin(), names.end(), "Employee 0-0") != names.end());
}

TEST(LazyQuerySliceAccess) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexTestData();
    
    // Test slice access
    auto gen = filter.queryGenerator(data, "$.large_array[0:5].name");
    std::vector<std::string> names;
    while (gen.hasNext()) {
        names.push_back(gen.next().value->toString());
    }
    
    ASSERT_EQ(5, names.size());
    ASSERT_EQ("Item_0", names[0]);
    ASSERT_EQ("Item_4", names[4]);
}

TEST(LazyQuerySpecialCharacters) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexTestData();
    
    // Test special character handling
    auto gen = filter.queryGenerator(data, "$['special_keys']['key-with-dash']");
    ASSERT_TRUE(gen.hasNext());
    auto queryResult = gen.next();
    ASSERT_EQ("dash_value", queryResult.value->toString());
    ASSERT_FALSE(gen.hasNext());
}

TEST(LazyQueryEmptyResults) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexTestData();
    
    // Test non-existent path
    auto gen = filter.queryGenerator(data, "$.nonexistent.path");
    ASSERT_FALSE(gen.hasNext());
}

TEST(LazyQueryMaxResults) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexTestData();
    
    // Test max results limitation
    auto gen = filter.queryGenerator(data, "$.large_array[*].name", 10);
    std::vector<std::string> names;
    while (gen.hasNext()) {
        names.push_back(gen.next().value->toString());
    }
    
    ASSERT_EQ(10, names.size());
}

TEST(LazyQueryBatchProcessing) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexTestData();
    
    // Test batch processing
    auto gen = filter.queryGenerator(data, "$.large_array[*].name", 50);
    auto batch = gen.nextBatch(25);
    
    ASSERT_EQ(25, batch.size());
    ASSERT_TRUE(gen.hasNext());
    
    auto secondBatch = gen.nextBatch(25);
    ASSERT_EQ(25, secondBatch.size());
    ASSERT_FALSE(gen.hasNext());
}

TEST(LazyQueryConsistencyWithTraditional) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexTestData();
    
    std::string expression = "$.company.departments[*].employees[*].name";
    
    // Get results using traditional method
    auto traditionalResults = filter.query(data, expression);
    
    // Get results using lazy method
    auto gen = filter.queryGenerator(data, expression);
    std::vector<QueryResult> lazyResults;
    while (gen.hasNext()) {
        lazyResults.push_back(gen.next());
    }
    
    ASSERT_EQ(traditionalResults.size(), lazyResults.size());
    
    // Compare values (order might differ for some queries)
    std::vector<std::string> traditionalValues, lazyValues;
    for (const auto& queryResult : traditionalResults) {
        traditionalValues.push_back(queryResult.value->toString());
    }
    for (const auto& queryResult : lazyResults) {
        lazyValues.push_back(queryResult.value->toString());
    }
    
    std::sort(traditionalValues.begin(), traditionalValues.end());
    std::sort(lazyValues.begin(), lazyValues.end());
    
    ASSERT_TRUE(traditionalValues == lazyValues);
}

// Performance tests
TEST(LazyQueryPerformanceSmallResults) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeBookData(1000);
    
    PerformanceTimer timer;
    
    // Test lazy loading for small result set (should use fast path)
    timer.start();
    auto gen = filter.queryGenerator(data, "$.store.book[0:10].title");
    std::vector<QueryResult> results;
    while (gen.hasNext()) {
        results.push_back(gen.next());
    }
    long long lazyTime = timer.end();
    
    // Test traditional method
    timer.start();
    auto traditionalResults = filter.query(data, "$.store.book[0:10].title");
    long long traditionalTime = timer.end();
    
    ASSERT_EQ(10, results.size());
    ASSERT_EQ(traditionalResults.size(), results.size());
    
    // For small results, lazy should be competitive or better
    std::cout << "Small results - Lazy: " << lazyTime << "μs, Traditional: " << traditionalTime << "μs" << std::endl;
}

TEST(LazyQueryPerformanceLargeResults) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeBookData(1000);
    
    PerformanceTimer timer;
    
    // Test lazy loading for large result set
    timer.start();
    auto gen = filter.queryGenerator(data, "$.store.book[*].title", 500);
    std::vector<QueryResult> results;
    while (gen.hasNext()) {
        results.push_back(gen.next());
    }
    long long lazyTime = timer.end();
    
    // Test traditional method
    timer.start();
    auto traditionalResults = filter.query(data, "$.store.book[*].title");
    long long traditionalTime = timer.end();
    
    ASSERT_EQ(500, results.size());
    ASSERT_EQ(1000, traditionalResults.size());
    
    std::cout << "Large results - Lazy (500): " << lazyTime << "μs, Traditional (1000): " << traditionalTime << "μs" << std::endl;
}

TEST(LazyQueryPerformanceBatchSizes) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeBookData(1000);
    
    std::vector<size_t> batchSizes = {10, 25, 50, 100};
    
    for (size_t batchSize : batchSizes) {
        PerformanceTimer timer;
        timer.start();
        
        auto gen = filter.queryGenerator(data, "$.store.book[*].title", 200);
        std::vector<QueryResult> allResults;
        
        while (gen.hasNext() && allResults.size() < 200) {
            auto batch = gen.nextBatch(batchSize);
            allResults.insert(allResults.end(), batch.begin(), batch.end());
            
            if (batch.size() < batchSize) {
                break;
            }
        }
        
        long long totalTime = timer.end();
        
        ASSERT_TRUE(allResults.size() <= 200);
        std::cout << "Batch size " << batchSize << ": " << totalTime << "μs, Results: " << allResults.size() << std::endl;
    }
}

TEST(LazyQueryMemoryEfficiency) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = generateLargeBookData(10000); // Large dataset
    
    // Test that lazy loading doesn't load all results into memory at once
    auto gen = filter.queryGenerator(data, "$.store.book[*].title", 100);
    
    // Get first result
    ASSERT_TRUE(gen.hasNext());
    auto firstResult = gen.next();
    ASSERT_EQ("Book 1", firstResult.value->toString());
    
    // Should still have more results available
    ASSERT_TRUE(gen.hasNext());
    
    // Get remaining results in small batches
    int totalResults = 1;
    while (gen.hasNext() && totalResults < 100) {
        auto batch = gen.nextBatch(10);
        totalResults += batch.size();
        if (batch.size() < 10) break;
    }
    
    ASSERT_EQ(100, totalResults);
}

TEST(LazyQueryErrorHandling) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexTestData();
    
    // Test that invalid paths just return empty results (this is the current behavior)
    auto gen1 = filter.queryGenerator(data, "$.nonexistent.path");
    ASSERT_FALSE(gen1.hasNext());
    
    // Test another non-existent path
    auto gen2 = filter.queryGenerator(data, "$.company.nonexistent");
    ASSERT_FALSE(gen2.hasNext());
    
    // Test valid path for comparison
    auto gen3 = filter.queryGenerator(data, "$.company.name");
    ASSERT_TRUE(gen3.hasNext());
}

TEST(LazyQueryNestedArrays) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexTestData();
    
    // Test nested array access
    auto gen = filter.queryGenerator(data, "$.company.departments[*].employees[*].skills[*]");
    std::vector<std::string> skills;
    while (gen.hasNext()) {
        skills.push_back(gen.next().value->toString());
    }
    
    // 3 departments * 5 employees * 3 skills = 45
    ASSERT_EQ(45, skills.size());
    
    // Verify some expected values
    ASSERT_TRUE(std::find(skills.begin(), skills.end(), "Skill0") != skills.end());
    ASSERT_TRUE(std::find(skills.begin(), skills.end(), "Skill1") != skills.end());
    ASSERT_TRUE(std::find(skills.begin(), skills.end(), "Skill2") != skills.end());
}

TEST(LazyQueryComplexFiltering) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexTestData();
    
    // Test complex path with multiple conditions
    auto gen = filter.queryGenerator(data, "$.company.departments[*].employees[*].address.city");
    std::vector<std::string> cities;
    while (gen.hasNext()) {
        cities.push_back(gen.next().value->toString());
    }
    
    // 3 departments * 5 employees = 15 cities
    ASSERT_EQ(15, cities.size());
    
    // Verify pattern (employees in department 0 are in City 0, etc.)
    ASSERT_TRUE(std::count(cities.begin(), cities.end(), "City 0") == 5);
    ASSERT_TRUE(std::count(cities.begin(), cities.end(), "City 1") == 5);
    ASSERT_TRUE(std::count(cities.begin(), cities.end(), "City 2") == 5);
}

int main() {
    return RUN_ALL_TESTS();
}
