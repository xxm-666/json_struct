// JSONPath 可变过滤器测试
#include "../src/jsonstruct.h"
#include "json_engine/json_path.h"
#include "../test_framework/test_framework.h"
#include <iostream>

using namespace JsonStruct;
using namespace JsonStruct::literals;

// Test mutable filter operations
TEST(JsonPath_MutableFilter) {
    JsonValue json = JsonValue::parse(R"({
        "users": [
            {"name": "Alice", "age": 25, "active": true},
            {"name": "Bob", "age": 30, "active": false},
            {"name": "Charlie", "age": 22, "active": true}
        ]
    })");

    // Test mutable filtering
    auto mutableResults = jsonvalue_jsonpath::selectAllMutable(json, "$.users[?(@.age > 24)]");
    ASSERT_EQ(mutableResults.size(), 2); // Alice and Bob
    
    // Modify the mutable results
    for (auto& result : mutableResults) {
        if (result.get()["name"].toString() == "Alice") {
            result.get()["age"] = JsonValue(26);
        } else if (result.get()["name"].toString() == "Bob") {
            result.get()["age"] = JsonValue(31);
        }
    }
    
    // Verify changes were applied
    auto alice = jsonvalue_jsonpath::selectFirst(json, "$.users[?(@.name == 'Alice')]");
    ASSERT_TRUE(alice.has_value());
    ASSERT_EQ(alice.value().get()["age"].toInt(), 26);
    
    auto bob = jsonvalue_jsonpath::selectFirst(json, "$.users[?(@.name == 'Bob')]");
    ASSERT_TRUE(bob.has_value());
    ASSERT_EQ(bob.value().get()["age"].toInt(), 31);
}

// Test nested mutable filter operations
TEST(JsonPath_MutableNestedFilter) {
    JsonValue json = JsonValue::parse(R"({
        "departments": [
            {
                "name": "Engineering",
                "employees": [
                    {"name": "Alice", "salary": 80000},
                    {"name": "Bob", "salary": 75000}
                ]
            },
            {
                "name": "Marketing", 
                "employees": [
                    {"name": "Charlie", "salary": 60000},
                    {"name": "David", "salary": 65000}
                ]
            }
        ]
    })");

    // Test nested mutable filtering - get high salary employees
    auto highSalaryEmployees = jsonvalue_jsonpath::selectAllMutable(json, "$.departments[*].employees[?(@.salary > 70000)]");
    ASSERT_EQ(highSalaryEmployees.size(), 2); // Alice and Bob
    
    // Give them a raise
    for (auto& employee : highSalaryEmployees) {
        int currentSalary = employee.get()["salary"].toInt();
        employee.get()["salary"] = JsonValue(currentSalary + 5000);
    }
    
    // Verify the raises were applied
    auto alice = jsonvalue_jsonpath::selectFirst(json, "$..employees[?(@.name == 'Alice')]");
    ASSERT_TRUE(alice.has_value());
    ASSERT_EQ(alice.value().get()["salary"].toInt(), 85000);
    
    auto bob = jsonvalue_jsonpath::selectFirst(json, "$..employees[?(@.name == 'Bob')]");
    ASSERT_TRUE(bob.has_value());
    ASSERT_EQ(bob.value().get()["salary"].toInt(), 80000);
    
    // Verify others were not affected
    auto charlie = jsonvalue_jsonpath::selectFirst(json, "$..employees[?(@.name == 'Charlie')]");
    ASSERT_TRUE(charlie.has_value());
    ASSERT_EQ(charlie.value().get()["salary"].toInt(), 60000);
}

// Test complex nested filter conditions with mutable operations
TEST(JsonPath_MutableComplexFilter) {
    JsonValue json = JsonValue::parse(R"({
        "orders": [
            {
                "id": 1,
                "items": [
                    {"name": "apple", "price": 120, "quantity": 2},
                    {"name": "banana", "price": 30, "quantity": 5}
                ]
            },
            {
                "id": 2,
                "items": [
                    {"name": "orange", "price": 150, "quantity": 1}
                ]
            }
        ]
    })");

    // First test the read-only version to verify filtering works
    auto readOnlyResults = jsonvalue_jsonpath::selectAll(json, "$.orders[?(@.items[?(@.price > 100)])]");
    ASSERT_EQ(readOnlyResults.size(), 2); // Both orders have expensive items
    
    // Test complex nested filter with mutable access - expensive items
    auto expensiveItems = jsonvalue_jsonpath::selectAllMutable(json, "$.orders[?(@.items[?(@.price > 100)])]");
    ASSERT_EQ(expensiveItems.size(), 2); // Both orders have expensive items
    
    // Apply discount to expensive orders
    for (auto& order : expensiveItems) {
        // Add a discount field to orders with expensive items
        order.get()["discount"] = JsonValue(10); // 10% discount
    }
    
    // Verify discounts were applied by checking the modified JSON
    ASSERT_TRUE(json["orders"][0].contains("discount"));
    ASSERT_EQ(json["orders"][0]["discount"].toInt(), 10);
    
    ASSERT_TRUE(json["orders"][1].contains("discount"));
    ASSERT_EQ(json["orders"][1]["discount"].toInt(), 10);
}

// Test mutable operations with logical operators
TEST(JsonPath_MutableLogicalFilter) {
    JsonValue json = JsonValue::parse(R"({
        "products": [
            {"name": "Widget A", "price": 10, "stock": 100, "category": "electronics"},
            {"name": "Widget B", "price": 25, "stock": 50, "category": "electronics"},
            {"name": "Book C", "price": 15, "stock": 200, "category": "books"},
            {"name": "Gadget D", "price": 8, "stock": 5, "category": "electronics"}
        ]
    })");

    // Test logical AND with mutable results - expensive electronics with good stock
    auto expensiveElectronics = jsonvalue_jsonpath::selectAllMutable(json, 
        "$.products[?(@.category == 'electronics' && @.price > 15 && @.stock > 40)]");
    ASSERT_EQ(expensiveElectronics.size(), 1); // Only Widget B
    
    // Mark as featured
    for (auto& product : expensiveElectronics) {
        product.get()["featured"] = JsonValue(true);
    }
    
    // Verify Widget B is now featured
    auto widgetB = jsonvalue_jsonpath::selectFirst(json, "$.products[?(@.name == 'Widget B')]");
    ASSERT_TRUE(widgetB.has_value());
    ASSERT_TRUE(widgetB.value().get().contains("featured"));
    ASSERT_EQ(widgetB.value().get()["featured"].toBool(), true);
    
    // Test logical OR with mutable results - low stock or cheap items
    auto specialOffers = jsonvalue_jsonpath::selectAllMutable(json, 
        "$.products[?(@.stock < 10 || @.price < 12)]");
    ASSERT_EQ(specialOffers.size(), 2); // Widget A and Gadget D
    
    // Add special offer flag
    for (auto& product : specialOffers) {
        product.get()["special_offer"] = JsonValue(true);
    }
    
    // Verify special offers
    auto widgetA = jsonvalue_jsonpath::selectFirst(json, "$.products[?(@.name == 'Widget A')]");
    ASSERT_TRUE(widgetA.has_value());
    ASSERT_TRUE(widgetA.value().get().contains("special_offer"));
    
    auto gadgetD = jsonvalue_jsonpath::selectFirst(json, "$.products[?(@.name == 'Gadget D')]");
    ASSERT_TRUE(gadgetD.has_value());
    ASSERT_TRUE(gadgetD.value().get().contains("special_offer"));
}

int main(int argc, char** argv) {
    return RUN_ALL_TESTS();
}
