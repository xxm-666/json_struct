#include "jsonstruct.h"
#include "json_engine/json_path.h"
#include "test_framework.h"
#include <iostream>

using namespace JsonStruct;

// Test multi-level nested filter expressions
TEST(JsonPath_MultiLevelNestedFilters) {
    JsonValue json = JsonValue::parse(R"({
        "companies": [
            {
                "name": "TechCorp",
                "departments": [
                    {
                        "name": "Engineering",
                        "teams": [
                            {
                                "name": "Backend",
                                "members": [
                                    {"name": "Alice", "age": 30, "role": "Senior"},
                                    {"name": "Bob", "age": 25, "role": "Junior"}
                                ]
                            },
                            {
                                "name": "Frontend",
                                "members": [
                                    {"name": "Charlie", "age": 32, "role": "Senior"},
                                    {"name": "Dave", "age": 27, "role": "Mid"}
                                ]
                            }
                        ]
                    },
                    {
                        "name": "Sales",
                        "teams": [
                            {
                                "name": "Enterprise",
                                "members": [
                                    {"name": "Eve", "age": 35, "role": "Manager"},
                                    {"name": "Frank", "age": 29, "role": "Rep"}
                                ]
                            }
                        ]
                    }
                ]
            },
            {
                "name": "StartupInc",
                "departments": [
                    {
                        "name": "Development",
                        "teams": [
                            {
                                "name": "Full Stack",
                                "members": [
                                    {"name": "Grace", "age": 26, "role": "Developer"},
                                    {"name": "Henry", "age": 31, "role": "Lead"}
                                ]
                            }
                        ]
                    }
                ]
            }
        ]
    })");

    // Test 4-level nested filter: Find companies that have departments with teams with members over 28
    auto result1 = jsonpath::JsonPath::parse("$.companies[?(@.departments[?(@.teams[?(@.members[?(@.age > 28)])])])]").selectAll(json);
    
    std::cout << "Companies with members over 28: " << result1.size() << std::endl;
    for (const auto& company : result1) {
        std::cout << "  - " << company.get()["name"].toString() << std::endl;
    }
    
    // Should find both companies (TechCorp has Alice(30), Charlie(32), Eve(35), Henry(31); StartupInc has Henry(31))
    ASSERT_EQ(result1.size(), 2u);
    ASSERT_EQ(result1[0].get()["name"].toString(), "TechCorp");
    ASSERT_EQ(result1[1].get()["name"].toString(), "StartupInc");
    
    // Test with more specific condition: Find companies with Senior members over 30
    auto result2 = jsonpath::JsonPath::parse("$.companies[?(@.departments[?(@.teams[?(@.members[?(@.age > 30 && @.role == 'Senior')])])])]").selectAll(json);
    
    std::cout << "Companies with Senior members over 30: " << result2.size() << std::endl;
    for (const auto& company : result2) {
        std::cout << "  - " << company.get()["name"].toString() << std::endl;
    }
    
    // Should find only TechCorp (has Charlie who is 32 and Senior)
    ASSERT_EQ(result2.size(), 1u);
    ASSERT_EQ(result2[0].get()["name"].toString(), "TechCorp");
    
    // Test 3-level nested filter: Find departments with teams having members over 30
    auto result3 = jsonpath::JsonPath::parse("$.companies[*].departments[?(@.teams[?(@.members[?(@.age > 30)])])]").selectAll(json);
    
    std::cout << "Departments with members over 30: " << result3.size() << std::endl;
    for (const auto& dept : result3) {
        std::cout << "  - " << dept.get()["name"].toString() << std::endl;
    }
    
    // Should find Engineering (has Alice 30, Charlie 32), Sales (has Eve 35), and Development (has Henry 31)
    ASSERT_TRUE(result3.size() >= 2u); // At least Engineering and Sales from TechCorp, plus Development from StartupInc
}

// Test nested filters with different operators
TEST(JsonPath_NestedFiltersWithOperators) {
    JsonValue json = JsonValue::parse(R"({
        "stores": [
            {
                "name": "Store A",
                "products": [
                    {
                        "category": "electronics",
                        "items": [
                            {"name": "laptop", "price": 1200, "rating": 4.5},
                            {"name": "phone", "price": 800, "rating": 4.2}
                        ]
                    },
                    {
                        "category": "books",
                        "items": [
                            {"name": "novel", "price": 15, "rating": 4.8},
                            {"name": "textbook", "price": 120, "rating": 4.0}
                        ]
                    }
                ]
            },
            {
                "name": "Store B", 
                "products": [
                    {
                        "category": "electronics",
                        "items": [
                            {"name": "tablet", "price": 600, "rating": 4.3},
                            {"name": "headphones", "price": 200, "rating": 4.7}
                        ]
                    }
                ]
            }
        ]
    })");
    
    // Find stores with expensive electronics (price > 500 in electronics category)
    auto result1 = jsonpath::JsonPath::parse("$.stores[?(@.products[?(@.category == 'electronics' && @.items[?(@.price > 500)])])]").selectAll(json);
    
    std::cout << "Stores with expensive electronics: " << result1.size() << std::endl;
    for (const auto& store : result1) {
        std::cout << "  - " << store.get()["name"].toString() << std::endl;
    }
    
    // Should find both stores (Store A has laptop 1200, phone 800; Store B has tablet 600)
    ASSERT_EQ(result1.size(), 2u);
    
    // Find stores with highly rated items (rating > 4.6)
    auto result2 = jsonpath::JsonPath::parse("$.stores[?(((@.products[?(@.items[?(@.rating > 4.6)])])))]").selectAll(json);
    
    std::cout << "Stores with highly rated items: " << result2.size() << std::endl;
    for (const auto& store : result2) {
        std::cout << "  - " << store.get()["name"].toString() << std::endl;
    }
    
    // Should find both stores (Store A has novel 4.8; Store B has headphones 4.7)
    ASSERT_EQ(result2.size(), 2u);

	auto checkBroken = jsonvalue_jsonpath::selectAll(json, "$.stores[?(@.products[?(@.items[?(@.price > 50 && @.rating > 3.5)])])]");
	ASSERT_EQ(checkBroken.size(), 2u); // Both stores have items that match the condition
}

TEST(JsonPath_UnionExpressions) {
    JsonValue json = JsonValue::parse(R"({
        "data": {
            "primary": {"value": 100},
            "secondary": {"value": 200}, 
            "tertiary": {"value": 300},
            "items": [1, 2, 3, 4, 5]
        }
    })");

    auto unionResult = jsonvalue_jsonpath::selectAllMutable(json, "$.data.primary.value,$.data.secondary.value");
    ASSERT_EQ(unionResult.size(), 2);
    ASSERT_EQ(unionResult[0].get().toInt(), 100);
    ASSERT_EQ(unionResult[1].get().toInt(), 200);

    auto multiIndex = jsonvalue_jsonpath::selectAllMutable(json, "$.data.items[0,2,4]");
    ASSERT_EQ(multiIndex.size(), 3);
    ASSERT_EQ(multiIndex[0].get().toInt(), 1);
    ASSERT_EQ(multiIndex[1].get().toInt(), 3);
    ASSERT_EQ(multiIndex[2].get().toInt(), 5);
}

int main(int argc, char** argv) {
    return RUN_ALL_TESTS();
}
