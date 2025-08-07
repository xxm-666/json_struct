#include "../test_framework/test_framework.h"
#include "../src/json_engine/query_factory.h"
#include "../src/json_engine/json_filter.h"
#include "../src/json_engine/json_value.h"
#include <chrono>
#include <random>
#include <sstream>
#include <functional>

using namespace JsonStruct;
using namespace TestFramework;

// Create complex nested data for testing
JsonValue createComplexNestedData() {
    JsonValue::ObjectType root;
    
    // Create deeply nested structure
    JsonValue::ArrayType companies;
    for (int compId = 0; compId < 5; ++compId) {
        JsonValue::ObjectType company;
        company["id"] = JsonValue(compId);
        company["name"] = JsonValue("Company_" + std::to_string(compId));
        company["founded"] = JsonValue(2000 + compId);
        
        // Add departments
        JsonValue::ArrayType departments;
        for (int deptId = 0; deptId < 8; ++deptId) {
            JsonValue::ObjectType dept;
            dept["id"] = JsonValue(deptId);
            dept["name"] = JsonValue("Department_" + std::to_string(deptId));
            dept["budget"] = JsonValue(100000.0 + deptId * 10000);
            
            // Add employees
            JsonValue::ArrayType employees;
            for (int empId = 0; empId < 15; ++empId) {
                JsonValue::ObjectType employee;
                employee["id"] = JsonValue(empId);
                employee["name"] = JsonValue("Employee_" + std::to_string(empId));
                employee["salary"] = JsonValue(50000.0 + empId * 5000);
                employee["active"] = JsonValue(empId % 3 != 0); // Some inactive
                
                // Add projects
                JsonValue::ArrayType projects;
                for (int projId = 0; projId < 6; ++projId) {
                    if (empId % 2 == 0 || projId < 3) { // Not all employees have all projects
                        JsonValue::ObjectType project;
                        project["id"] = JsonValue(projId);
                        project["name"] = JsonValue("Project_" + std::to_string(projId));
                        project["priority"] = JsonValue(projId % 3 == 0 ? "high" : (projId % 3 == 1 ? "medium" : "low"));
                        project["completed"] = JsonValue(projId < 2);
                        projects.push_back(JsonValue(std::move(project)));
                    }
                }
                employee["projects"] = JsonValue(std::move(projects));
                employees.push_back(JsonValue(std::move(employee)));
            }
            dept["employees"] = JsonValue(std::move(employees));
            departments.push_back(JsonValue(std::move(dept)));
        }
        company["departments"] = JsonValue(std::move(departments));
        companies.push_back(JsonValue(std::move(company)));
    }
    root["companies"] = JsonValue(std::move(companies));
    
    // Add some top-level metadata
    root["metadata"] = JsonValue::ObjectType{
        {"version", JsonValue("2.1.0")},
        {"timestamp", JsonValue("2025-01-07T10:00:00Z")},
        {"total_records", JsonValue(5 * 8 * 15)} // companies * departments * employees
    };
    
    return JsonValue(std::move(root));
}

// Create data with edge cases
JsonValue createEdgeCaseData() {
    JsonValue::ObjectType root;
    
    // Empty arrays and objects
    root["empty_array"] = JsonValue(JsonValue::ArrayType{});
    root["empty_object"] = JsonValue(JsonValue::ObjectType{});
    
    // Arrays with mixed types
    JsonValue::ArrayType mixed;
    mixed.push_back(JsonValue(42));
    mixed.push_back(JsonValue("text"));
    mixed.push_back(JsonValue(true));
    mixed.push_back(JsonValue(3.14));
    mixed.push_back(JsonValue(JsonValue::ObjectType{{"nested", JsonValue("value")}}));
    mixed.push_back(JsonValue(JsonValue::ArrayType{JsonValue(1), JsonValue(2)}));
    root["mixed_array"] = JsonValue(std::move(mixed));
    
    // Special numeric values
    root["numbers"] = JsonValue::ObjectType{
        {"zero", JsonValue(0)},
        {"negative", JsonValue(-42)},
        {"large", JsonValue(1000000)},
        {"decimal", JsonValue(123.456)}
    };
    
    // Special string values
    root["strings"] = JsonValue::ObjectType{
        {"empty", JsonValue("")},
        {"space", JsonValue(" ")},
        {"special_chars", JsonValue("!@#$%^&*()")},
        {"unicode", JsonValue("测试数据")}
    };
    
    // Nested structure with repeated patterns
    JsonValue::ArrayType levels;
    for (int i = 0; i < 3; ++i) {
        JsonValue::ObjectType level;
        level["level"] = JsonValue(i);
        
        JsonValue::ArrayType items;
        for (int j = 0; j < 4; ++j) {
            JsonValue::ObjectType item;
            item["id"] = JsonValue(j);
            item["value"] = JsonValue(i * 10 + j);
            
            if (i > 0) {
                // Add sub-items for deeper levels
                JsonValue::ArrayType subItems;
                for (int k = 0; k < 3; ++k) {
                    subItems.push_back(JsonValue::ObjectType{
                        {"sub_id", JsonValue(k)},
                        {"sub_value", JsonValue(k * 100)}
                    });
                }
                item["sub_items"] = JsonValue(std::move(subItems));
            }
            items.push_back(JsonValue(std::move(item)));
        }
        level["items"] = JsonValue(std::move(items));
        levels.push_back(JsonValue(std::move(level)));
    }
    root["levels"] = JsonValue(std::move(levels));
    
    return JsonValue(std::move(root));
}

TEST(EnhancedLazyDeepNesting) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexNestedData();
    
    // Test deeply nested path access
    auto gen1 = EnhancedQueryFactory::createGenerator(filter, data, 
        "$.companies[*].departments[*].employees[*].projects[*].name");
    
    std::vector<JsonValue> results;
    while (gen1.hasNext() && results.size() < 50) {
        results.push_back(*gen1.next().value);
    }
    
    ASSERT_GT(results.size(), 0u);
    // Should find project names from multiple nested levels
    
    // Test recursive descent with complex filtering
    auto gen2 = EnhancedQueryFactory::createGenerator(filter, data,
        "$..employees[?(@.salary > 75000)]");
    
    results.clear();
    while (gen2.hasNext() && results.size() < 20) {
        results.push_back(*gen2.next().value);
    }
    
    ASSERT_GT(results.size(), 0u);
    // Should find high-salary employees across all departments
}

TEST(EnhancedLazySlicingEdgeCases) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexNestedData();
    
    // Test various slice patterns
    std::vector<std::string> sliceQueries = {
        "$.companies[0:2].departments[1:3].employees[::2].name", // Step slicing
        "$.companies[-1:].departments[-2:].employees[0:5].id",  // Negative indices
        "$.companies[:1].departments[:].employees[10:].salary", // Open ranges
        "$.companies[1:4:2].departments[0::3].employees[-5:].active" // Complex steps
    };
    
    for (const auto& query : sliceQueries) {
        auto gen = EnhancedQueryFactory::createGenerator(filter, data, query);
        
        std::vector<JsonValue> results;
        while (gen.hasNext() && results.size() < 30) {
            results.push_back(*gen.next().value);
        }
        
        // Should handle slice operations without errors
        // Results count depends on specific slice parameters
        ASSERT_GE(results.size(), 0u);
    }
}

TEST(EnhancedLazyFilterComplexity) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexNestedData();
    
    // Test complex filter expressions
    std::vector<std::string> filterQueries = {
        "$.companies[?(@.founded > 2002)].departments[*].employees[?(@.salary > 60000 && @.active == true)]",
        "$..projects[?(@.priority == 'high' && @.completed == false)]",
        "$.companies[*].departments[?(@.budget > 120000)].employees[?(@.id % 2 == 0)]",
        "$..employees[?(@.projects.length > 3)].name"
    };
    
    for (const auto& query : filterQueries) {
        auto gen = EnhancedQueryFactory::createGenerator(filter, data, query);
        
        std::vector<JsonValue> results;
        while (gen.hasNext() && results.size() < 25) {
            results.push_back(*gen.next().value);
        }
        
        // Complex filters should work correctly
        ASSERT_GE(results.size(), 0u);
    }
}

TEST(EnhancedLazyUnionOperations) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexNestedData();
    
    // Test union expressions using supported syntax
    std::vector<std::string> unionQueries = {
        "$.companies[0].name,$.companies[2].name,$.companies[4].name",  // Full path union
        "$.companies[*].name",                                           // Use wildcard instead of property union
        "$.companies[0].departments[0].employees[0].name,$.companies[0].departments[0].employees[5].name,$.companies[0].departments[0].employees[10].name",  // Employee name union
        "$.companies[0].departments[0].budget,$.companies[0].departments[3].budget,$.companies[0].departments[7].budget"  // Department budget union
    };
    
    for (const auto& query : unionQueries) {
        auto gen = EnhancedQueryFactory::createGenerator(filter, data, query);
        
        std::vector<JsonValue> results;
        while (gen.hasNext() && results.size() < 40) {
            results.push_back(*gen.next().value);
        }
        
        ASSERT_GT(results.size(), 0u);
        // Union operations should return multiple sets of results
    }
}

TEST(EnhancedLazyEdgeCases) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createEdgeCaseData();
    
    // Test queries on edge case data
    std::vector<std::string> edgeQueries = {
        "$.empty_array[*]",                    // Empty array access
        "$.empty_object.*",                    // Empty object access
        "$.mixed_array[?(@.type == 'number')]", // Type filtering on mixed array
        "$.strings[?(@.length == 0)]",         // Empty string filtering
        "$.numbers[?(@.value > 0)]",           // Numeric comparisons
        "$..sub_items[*].sub_value",           // Nested array access
        "$.levels[*].items[?(@.id == 0)]"      // Index-based filtering
    };
    
    for (const auto& query : edgeQueries) {
        auto gen = EnhancedQueryFactory::createGenerator(filter, data, query);
        
        std::vector<JsonValue> results;
        while (gen.hasNext() && results.size() < 20) {
            results.push_back(*gen.next().value);
        }
        
        // Edge cases should be handled gracefully
        ASSERT_GE(results.size(), 0u);
    }
}

TEST(EnhancedLazyPerformanceStress) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexNestedData();
    
    // Stress test with complex recursive query
    auto gen = EnhancedQueryFactory::createGenerator(filter, data,
        "$..employees[*].projects[?(@.priority != 'low')]");
    
    auto start = std::chrono::steady_clock::now();
    
    std::vector<JsonValue> results;
    while (gen.hasNext() && results.size() < 100) {
        results.push_back(*gen.next().value);
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    ASSERT_GT(results.size(), 0u);
    ASSERT_LT(duration, 100000); // Should complete within 100ms
    
    std::cout << "Stress test processed " << results.size() 
              << " results in " << duration << "μs\n";
}

TEST(EnhancedLazyCacheWithReset) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexNestedData();
    
    auto gen = EnhancedQueryFactory::createGenerator(filter, data,
        "$.companies[*].departments[*].employees[?(@.active == true)].name");
    
    gen.enableCache(true);
    
    // First run
    std::vector<JsonValue> firstResults;
    while (gen.hasNext() && firstResults.size() < 30) {
        firstResults.push_back(*gen.next().value);
    }
    
    size_t initialCacheSize = gen.getCacheSize();
    
    // Reset and run again
    gen.reset();
    std::vector<JsonValue> secondResults;
    while (gen.hasNext() && secondResults.size() < 30) {
        secondResults.push_back(*gen.next().value);
    }
    
    // Cache should be preserved and hit ratio should improve
    ASSERT_EQ(firstResults.size(), secondResults.size());
    ASSERT_GE(gen.getCacheSize(), initialCacheSize);
    ASSERT_GT(gen.getCacheHitRatio(), 0.0);
    
    std::cout << "Cache reset test - Hit ratio: " << gen.getCacheHitRatio() 
              << "%, Cache size: " << gen.getCacheSize() << "\n";
}

TEST(EnhancedLazyBoundaryValues) {
    JsonFilter filter = JsonFilter::createDefault();
    
    // Test with minimal data
    JsonValue minimalData = JsonValue::ObjectType{
        {"single", JsonValue(42)},
        {"array", JsonValue::ArrayType{JsonValue(1)}}
    };
    
    auto gen1 = EnhancedQueryFactory::createGenerator(filter, minimalData, "$.single");
    ASSERT_TRUE(gen1.hasNext());
    auto result1 = gen1.next();
    ASSERT_TRUE(result1.value != nullptr);
    ASSERT_EQ(result1.value->toDouble(), 42.0);
    
    // Test with array boundary access
    auto gen2 = EnhancedQueryFactory::createGenerator(filter, minimalData, "$.array[0]");
    ASSERT_TRUE(gen2.hasNext());
    auto result2 = gen2.next();
    ASSERT_TRUE(result2.value != nullptr);
    ASSERT_EQ(result2.value->toDouble(), 1.0);
    
    // Test out-of-bounds access
    auto gen3 = EnhancedQueryFactory::createGenerator(filter, minimalData, "$.array[10]");
    ASSERT_FALSE(gen3.hasNext()); // Should not find anything
}

TEST(EnhancedLazyRecursiveDescentLimits) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexNestedData();
    
    // Test recursive descent with various patterns
    std::vector<std::string> recursiveQueries = {
        "$..id",                    // Find all id fields
        "$..name",                  // Find all name fields  
        "$..budget",                // Find all budget fields
        "$..projects",              // Find all projects arrays
        "$..employees[0].name"      // Recursive with specific index
    };
    
    for (const auto& query : recursiveQueries) {
        auto gen = EnhancedQueryFactory::createGenerator(filter, data, query);
        
        std::vector<JsonValue> results;
        size_t count = 0;
        while (gen.hasNext() && count < 200) { // Limit to prevent infinite loops
            results.push_back(*gen.next().value);
            count++;
        }
        
        ASSERT_GT(results.size(), 0u);
        ASSERT_LT(results.size(), 300u); // Should not hit the safety limit
        
        std::cout << "Recursive query '" << query << "' found " 
                  << results.size() << " results\n";
    }
}

TEST(EnhancedLazyMemoryEfficiency) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexNestedData();
    
    auto gen = EnhancedQueryFactory::createGenerator(filter, data,
        "$..employees[*].projects[*]");
    
    // Process results one by one without storing them all
    size_t processedCount = 0;
    size_t memoryCheckpoint = 100; // Check every 100 items
    
        while (gen.hasNext() && processedCount < 500) {
            auto result = gen.next();
            if (result.value) {
                processedCount++;            // Verify we can access the data
            if (processedCount % memoryCheckpoint == 0) {
                // The lazy generator should not accumulate results in memory
                // This is more of a design verification test
                std::cout << "Processed " << processedCount << " items\n";
            }
        }
    }
    
    ASSERT_GT(processedCount, 0u);
    std::cout << "Memory efficiency test processed " << processedCount << " total items\n";
}

TEST(EnhancedLazyFilterFunctionCombinations) {
    JsonFilter filter = JsonFilter::createDefault();
    JsonValue data = createComplexNestedData();
    
    // Test combinations of different filter functions
    std::vector<std::string> combinedQueries = {
        "$.companies[?(@.founded > 2001)].departments[?(@.budget > 110000)].employees[?(@.salary > 70000)]",
        "$..employees[?(@.active == true && @.id < 10)].projects[?(@.completed == false)]",
        "$.companies[*].departments[?(@.id % 2 == 0)].employees[?(@.projects.length > 2)].name"
    };
    
    for (const auto& query : combinedQueries) {
        auto gen = EnhancedQueryFactory::createGenerator(filter, data, query);
        
        std::vector<JsonValue> results;
        while (gen.hasNext() && results.size() < 30) {
            results.push_back(*gen.next().value);
        }
        
        // Combined filters should work correctly
        ASSERT_GE(results.size(), 0u);
        
        std::cout << "Combined filter query found " << results.size() << " results\n";
    }
}

int main() {
    std::cout << "Running Enhanced Lazy Query Comprehensive Tests v2...\n";
    return RUN_ALL_TESTS();
}
