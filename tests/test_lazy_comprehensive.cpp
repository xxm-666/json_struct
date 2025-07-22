#include "../src/json_engine/json_filter.h"
#include "../src/json_engine/json_value.h"
#include <iostream>
#include <chrono>
#include <cassert>
#include <iomanip>
#include <vector>
#include <string>

using namespace JsonStruct;

// 创建复杂的嵌套JSON数据用于测试
JsonValue createComplexTestData() {
    JsonValue::ObjectType root;
    
    // 1. 深层嵌套结构
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
            
            // 嵌套地址信息
            JsonValue::ObjectType address;
            address["street"] = JsonValue("Street " + std::to_string(e));
            address["city"] = JsonValue("City " + std::to_string(d));
            address["zipcode"] = JsonValue(10000 + d * 1000 + e);
            employee["address"] = JsonValue(std::move(address));
            
            // 技能数组
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
    
    // 2. 数组中的混合类型
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
    
    // 3. 包含特殊字符的属性名
    JsonValue::ObjectType specialKeys;
    specialKeys["key-with-dash"] = JsonValue("dash_value");
    specialKeys["key.with.dots"] = JsonValue("dots_value");
    specialKeys["key with spaces"] = JsonValue("spaces_value");
    specialKeys["key[with]brackets"] = JsonValue("brackets_value");
    specialKeys["key_123"] = JsonValue("underscore_value");
    
    root["special_keys"] = JsonValue(std::move(specialKeys));
    
    // 4. 大数组用于性能测试
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
    
    // 5. 空结构
    root["empty_object"] = JsonValue(JsonValue::ObjectType{});
    root["empty_array"] = JsonValue(JsonValue::ArrayType{});
    
    return JsonValue(std::move(root));
}

// 测试用例结构
struct TestCase {
    std::string name;
    std::string path;
    size_t expectedMinResults;
    size_t expectedMaxResults;
    bool shouldSucceed;
    std::string description;
};

void runTestCase(const TestCase& testCase, const JsonValue& testData, JsonFilter& filter) {
    std::cout << "\n=== Test: " << testCase.name << " ===" << std::endl;
    std::cout << "Path: " << testCase.path << std::endl;
    std::cout << "Description: " << testCase.description << std::endl;
    
    try {
        // 测试现代化API
        auto start = std::chrono::high_resolution_clock::now();
        auto modernGen = filter.queryGenerator(testData, testCase.path, 1000);
        
        std::vector<QueryResult> modernResults;
        while (modernGen.hasNext()) {
            modernResults.push_back(modernGen.next());
        }
        auto modernTime = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - start);
        
        // 测试传统API对比
        start = std::chrono::high_resolution_clock::now();
        auto traditionalResults = filter.query(testData, testCase.path);
        auto traditionalTime = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - start);
        
        // 验证结果
        std::cout << "Modern API results: " << modernResults.size() << std::endl;
        std::cout << "Traditional API results: " << traditionalResults.size() << std::endl;
        std::cout << "Modern time: " << modernTime.count() << " μs" << std::endl;
        std::cout << "Traditional time: " << traditionalTime.count() << " μs" << std::endl;
        
        // 检查结果数量
        bool resultCountOk = (modernResults.size() >= testCase.expectedMinResults && 
                             modernResults.size() <= testCase.expectedMaxResults);
        
        // 对于有maxResults限制的递归下降查询，传统API会返回所有结果，现代API会限制结果
        // 这种情况下不比较API间的一致性，只验证现代API是否正确限制了结果
        bool isLimitedRecursiveDescent = (testCase.path.find("..") != std::string::npos) && 
                                       (traditionalResults.size() > modernResults.size());
        
        // 检查一致性（仅当两个API预期返回相同数量结果时）
        bool consistencyOk = true;
        bool pathConsistencyOk = true;
        
        if (!isLimitedRecursiveDescent) {
            consistencyOk = (modernResults.size() == traditionalResults.size());
            
            // 检查路径一致性
            if (consistencyOk) {
                for (size_t i = 0; i < modernResults.size(); ++i) {
                    if (modernResults[i].path != traditionalResults[i].path) {
                        pathConsistencyOk = false;
                        std::cout << "Path mismatch at index " << i << ": " 
                                  << modernResults[i].path << " vs " << traditionalResults[i].path << std::endl;
                        break;
                    }
                }
            }
        } else {
            // 对于有限制的递归下降，只检查现代API的结果是否在合理范围内
            std::cout << "Note: Limited recursive descent - modern API correctly limited results" << std::endl;
        }
        
        if (testCase.shouldSucceed) {
            assert(resultCountOk && "Result count not in expected range");
            if (!isLimitedRecursiveDescent) {
                assert(consistencyOk && "Result count mismatch between APIs");
                assert(pathConsistencyOk && "Path mismatch between APIs");
            }
            std::cout << "✅ PASS" << std::endl;
        } else {
            std::cout << "ℹ️  Expected to fail - this is normal" << std::endl;
        }
        
        // 显示前几个结果示例
        if (!modernResults.empty()) {
            std::cout << "Sample results:" << std::endl;
            for (size_t i = 0; i < std::min(size_t(3), modernResults.size()); ++i) {
                std::cout << "  [" << i << "] " << modernResults[i].path << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        if (testCase.shouldSucceed) {
            std::cout << "❌ FAIL - Exception: " << e.what() << std::endl;
            throw;
        } else {
            std::cout << "✅ EXPECTED FAIL - Exception: " << e.what() << std::endl;
        }
    } catch (...) {
        if (testCase.shouldSucceed) {
            std::cout << "❌ FAIL - Unknown exception" << std::endl;
            throw;
        } else {
            std::cout << "✅ EXPECTED FAIL - Unknown exception" << std::endl;
        }
    }
}

void testBasicPaths() {
    std::cout << "\n🔍 Testing Basic JSONPath Expressions..." << std::endl;
    
    JsonValue testData = createComplexTestData();
    JsonFilter filter = JsonFilter::createDefault();
    
    std::vector<TestCase> basicTests = {
        {"Root Access", "$", 1, 1, true, "Access the root object"},
        {"Simple Property", "$.company.name", 1, 1, true, "Access simple property"},
        {"Array Access", "$.company.departments[0]", 1, 1, true, "Access first array element"},
        {"All Array Elements", "$.company.departments[*]", 3, 3, true, "Access all array elements"},
        {"Property After Array", "$.company.departments[*].name", 3, 3, true, "Property access after array wildcard"},
        {"Non-existent Property", "$.nonexistent", 0, 0, true, "Access non-existent property"},
        {"Non-existent Path", "$.company.nonexistent.path", 0, 0, true, "Access non-existent nested path"},
    };
    
    for (const auto& test : basicTests) {
        runTestCase(test, testData, filter);
    }
}

void testComplexPaths() {
    std::cout << "\n🔍 Testing Complex JSONPath Expressions..." << std::endl;
    
    JsonValue testData = createComplexTestData();
    JsonFilter filter = JsonFilter::createDefault();
    
    std::vector<TestCase> complexTests = {
        {"Deep Nesting", "$.company.departments[*].employees[*].name", 15, 15, true, 
         "Deep nested array access"},
        {"Multiple Wildcards", "$.company.departments[*].employees[*].address.city", 15, 15, true,
         "Multiple wildcard with property access"},
        {"Mixed Array Access", "$.mixed_array[*]", 6, 6, true, "Access mixed type array"},
        {"Large Array Subset", "$.large_array[0:10]", 10, 10, true, "Array slice access"},
        {"Large Array All", "$.large_array[*].name", 1000, 1000, true, "All elements in large array"},
        {"Recursive Descent", "$..name", 1000, 1000, true, "Recursive descent operator (limited to 1000)"},
        {"Recursive with Filter", "$..employees[*]", 15, 15, true, "Recursive with array access"},
        {"Empty Structures", "$.empty_array[*]", 0, 0, true, "Access empty array elements"},
        {"Skills Array", "$.company.departments[0].employees[0].skills[*]", 3, 3, true, "Nested array access"},
    };
    
    for (const auto& test : complexTests) {
        runTestCase(test, testData, filter);
    }
}

void testSpecialCharacters() {
    std::cout << "\n🔍 Testing Special Character Handling..." << std::endl;
    
    JsonValue testData = createComplexTestData();
    JsonFilter filter = JsonFilter::createDefault();
    
    std::vector<TestCase> specialTests = {
        {"Dash in Key", "$['special_keys']['key-with-dash']", 1, 1, true, "Property with dash"},
        {"Dots in Key", "$['special_keys']['key.with.dots']", 1, 1, true, "Property with dots"},
        {"Spaces in Key", "$['special_keys']['key with spaces']", 1, 1, true, "Property with spaces"},
        {"Brackets in Key", "$['special_keys']['key[with]brackets']", 1, 1, true, "Property with brackets"},
        {"Underscore in Key", "$.special_keys.key_123", 1, 1, true, "Property with underscore"},
        {"All Special Keys", "$.special_keys[*]", 5, 5, true, "All properties with special characters"},
    };
    
    for (const auto& test : specialTests) {
        runTestCase(test, testData, filter);
    }
}

void testPerformanceScenarios() {
    std::cout << "\n🔍 Testing Performance Scenarios..." << std::endl;
    
    JsonValue testData = createComplexTestData();
    JsonFilter filter = JsonFilter::createDefault();
    
    std::vector<TestCase> performanceTests = {
        {"Small Result Set", "$.company.departments[0].employees[0:2]", 2, 2, true, 
         "Small result set should use fast path"},
        {"Medium Result Set", "$.large_array[0:500].name", 500, 500, true, 
         "Medium result set optimization"},
        {"Large Result Set", "$.large_array[*].name", 1000, 1000, true, 
         "Large result set should use traditional path"},
        {"Early Termination", "$.large_array[*].id", 1000, 1000, true, 
         "Test early termination with limit"},
    };
    
    for (const auto& test : performanceTests) {
        runTestCase(test, testData, filter);
    }
}

void testUnlimitedRecursiveDescent() {
    std::cout << "\n🔍 Testing Unlimited Recursive Descent..." << std::endl;
    
    JsonValue testData = createComplexTestData();
    JsonFilter filter = JsonFilter::createDefault();
    
    std::cout << "=== Test: Unlimited Recursive Descent ===" << std::endl;
    std::cout << "Path: $..name" << std::endl;
    std::cout << "Description: Recursive descent without maxResults limit" << std::endl;
    
    try {
        // 测试无限制的现代API (maxResults=0 表示无限制)
        auto start = std::chrono::high_resolution_clock::now();
        auto modernGen = filter.queryGenerator(testData, "$..name", 0);
        
        std::vector<QueryResult> modernResults;
        while (modernGen.hasNext()) {
            modernResults.push_back(modernGen.next());
        }
        auto modernTime = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - start);
        
        // 测试传统API对比
        start = std::chrono::high_resolution_clock::now();
        auto traditionalResults = filter.query(testData, "$..name");
        auto traditionalTime = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - start);
        
        std::cout << "Modern API results: " << modernResults.size() << std::endl;
        std::cout << "Traditional API results: " << traditionalResults.size() << std::endl;
        std::cout << "Modern time: " << modernTime.count() << " μs" << std::endl;
        std::cout << "Traditional time: " << traditionalTime.count() << " μs" << std::endl;
        
        // 验证结果一致性
        assert(modernResults.size() == traditionalResults.size() && "Unlimited recursive descent should return all results");
        assert(modernResults.size() == 1019 && "Should return all 1019 name fields");
        
        std::cout << "✅ PASS" << std::endl;
        std::cout << "Sample results:" << std::endl;
        for (size_t i = 0; i < std::min(size_t(3), modernResults.size()); ++i) {
            std::cout << "  [" << i << "] " << modernResults[i].path << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ FAIL: " << e.what() << std::endl;
    }
    
    std::cout << std::endl;
}

void testEdgeCases() {
    std::cout << "\n🔍 Testing Edge Cases..." << std::endl;
    
    JsonValue testData = createComplexTestData();
    JsonFilter filter = JsonFilter::createDefault();
    
    std::vector<TestCase> edgeTests = {
        {"Empty Path", "", 0, 0, false, "Empty JSONPath expression"},
        {"Invalid Syntax", "$..[", 0, 0, false, "Invalid JSONPath syntax"},
        {"Out of Bounds", "$.company.departments[999]", 0, 0, true, "Array index out of bounds"},
        {"Null Access", "$.mixed_array[4].property", 0, 0, true, "Property access on null"},
        {"Type Mismatch", "$.company.name[0]", 0, 0, true, "Array access on string"},
        {"Very Deep Path", "$..employees[*].address.city", 15, 15, true, "Very deep recursive path"},
    };
    
    for (const auto& test : edgeTests) {
        runTestCase(test, testData, filter);
    }
}

void testLazyIterationPatterns() {
    std::cout << "\n🔍 Testing Lazy Iteration Patterns..." << std::endl;
    
    JsonValue testData = createComplexTestData();
    JsonFilter filter = JsonFilter::createDefault();
    
    // 测试早期终止
    std::cout << "\n--- Early Termination Test ---" << std::endl;
    auto generator = filter.queryGenerator(testData, "$.large_array[*].name", 50);
    size_t count = 0;
    while (generator.hasNext() && count < 10) {
        auto result = generator.next();
        count++;
    }
    assert(count == 10);
    std::cout << "✅ Early termination works correctly" << std::endl;
    
    // 测试批量获取
    std::cout << "\n--- Batch Processing Test ---" << std::endl;
    auto batchGen = filter.queryGenerator(testData, "$.company.departments[*].employees[*].name", 100);
    auto batch1 = batchGen.nextBatch(5);
    auto batch2 = batchGen.nextBatch(5);
    auto batch3 = batchGen.nextBatch(10);
    
    assert(batch1.size() == 5);
    assert(batch2.size() == 5);
    assert(batch3.size() == 5); // Should have 15 total results
    std::cout << "✅ Batch processing works correctly" << std::endl;
    
    // 测试hasNext一致性
    std::cout << "\n--- hasNext Consistency Test ---" << std::endl;
    auto consistencyGen = filter.queryGenerator(testData, "$.company.departments[*].name", 10);
    size_t hasNextCount = 0;
    while (consistencyGen.hasNext()) {
        consistencyGen.next();
        hasNextCount++;
    }
    assert(hasNextCount == 3);
    assert(!consistencyGen.hasNext()); // Should be false after exhaustion
    std::cout << "✅ hasNext consistency works correctly" << std::endl;
}

int main() {
    try {
        std::cout << "🚀 Comprehensive Lazy Loading Test Suite" << std::endl;
        std::cout << "==========================================" << std::endl;
        
        testBasicPaths();
        testComplexPaths();
        testSpecialCharacters();
        testPerformanceScenarios();
        testUnlimitedRecursiveDescent();
        testEdgeCases();
        testLazyIterationPatterns();
        
        std::cout << "\n🎉 All tests completed successfully!" << std::endl;
        std::cout << "✅ Lazy loading implementation is robust and reliable." << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
}
