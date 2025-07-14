#include "../src/jsonstruct.h"
#include "json_filter.h"
#include <iostream>
#include <cassert>

using namespace JsonStruct;

int main() {
    std::cout << "=== JSONPath Filter Expression Support Test ===\n";
    
    // Create test JSON data
    JsonValue root;
    root["products"][0]["name"] = JsonValue("Product A");
    root["products"][0]["price"] = JsonValue(15.99);
    root["products"][0]["category"] = JsonValue("electronics");
    
    root["products"][1]["name"] = JsonValue("Product B");
    root["products"][1]["price"] = JsonValue(8.50);
    root["products"][1]["category"] = JsonValue("books");
    
    root["products"][2]["name"] = JsonValue("Product C");
    root["products"][2]["price"] = JsonValue(25.00);
    root["products"][2]["category"] = JsonValue("electronics");
    
    std::cout << "Test data created successfully\n";
    
    // Test 1: Filter expressions enabled (default)
    std::cout << "\n=== Test 1: Filter expressions enabled ===\n";
    auto defaultFilter = JsonFilter::createDefault();
    std::cout << root.toJson(true) << std::endl;

    try {
        // Test price filter
        auto results = defaultFilter.selectAll(root, "$.products[?(@.price < 10)]");
        std::cout << "Found " << results.size() << " products with price < 10\n";
        assert(results.size() == 1);
        std::cout << "✅ Price filter test passed\n";
        
        // Test category filter
        results = defaultFilter.selectAll(root, "$.products[?(@.category == \"electronics\")]");
        std::cout << "Found " << results.size() << " electronics products\n";
        assert(results.size() == 2);
        std::cout << "✅ Category filter test passed\n";
        
        // Test greater than filter
        results = defaultFilter.selectAll(root, "$.products[?(@.price > 20)]");
        std::cout << "Found " << results.size() << " products with price > 20\n";
        assert(results.size() == 1);
        std::cout << "✅ Greater than filter test passed\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Filter expressions test failed: " << e.what() << "\n";
        return 1;
    }
    
    // Test 2: Filter expressions disabled
    std::cout << "\n=== Test 2: Filter expressions disabled ===\n";
    auto strictFilter = JsonFilter::createStrict();
    
    try {
        auto results = strictFilter.selectAll(root, "$.products[?(@.price < 10)]");
        std::cerr << "❌ Expected exception for disabled filter expressions, but got " << results.size() << " results\n";
        return 1;
    } catch (const std::runtime_error& e) {
        std::cout << "✅ Filter expressions disabled as expected: " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cerr << "❌ Unexpected exception type: " << e.what() << "\n";
        return 1;
    }
    
    // Test 3: Regular JSONPath expressions still work
    std::cout << "\n=== Test 3: Regular JSONPath expressions ===\n";
    try {
        auto results = strictFilter.selectAll(root, "$.products[*].name");
        assert(results.size() == 3);
        std::cout << "✅ Regular JSONPath expressions work: found " << results.size() << " product names\n";
        
        results = strictFilter.selectAll(root, "$.products[0].price");
        assert(results.size() == 1);
        std::cout << "✅ Index access works: found " << results.size() << " price value\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Regular JSONPath test failed: " << e.what() << "\n";
        return 1;
    }
    
    // Test 4: Complex filter expressions
    std::cout << "\n=== Test 4: Complex filter expressions ===\n";
    try {
        // Test with different operators
        auto results = defaultFilter.selectAll(root, "$.products[?(@.price >= 15)]");
        std::cout << "Found " << results.size() << " products with price >= 15\n";
        assert(results.size() == 2);
        
        results = defaultFilter.selectAll(root, "$.products[?(@.price <= 10)]");
        std::cout << "Found " << results.size() << " products with price <= 10\n";
        assert(results.size() == 1);
        
        results = defaultFilter.selectAll(root, "$.products[?(@.category != \"books\")]");
        std::cout << "Found " << results.size() << " non-book products\n";
        assert(results.size() == 2);
        
        std::cout << "✅ Complex filter expressions test passed\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Complex filter expressions test failed: " << e.what() << "\n";
        return 1;
    }
    
    std::cout << "\n🎉 All JSONPath filter expression tests passed!\n";
    std::cout << "\n=== Summary ===\n";
    std::cout << "✅ Filter expressions can be enabled/disabled via QueryOptions\n";
    std::cout << "✅ Supports comparison operators: ==, !=, <, >, <=, >=\n";
    std::cout << "✅ Works with both string and numeric comparisons\n";
    std::cout << "✅ Regular JSONPath expressions remain unaffected\n";
    std::cout << "✅ Proper error handling when expressions are disabled\n";
    
    return 0;
}
