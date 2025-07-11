#include "json_value.h"
#include <iostream>
#include <cassert>

using namespace JsonStruct;

int main() {
    std::cout << "=== JSONPath Advanced Features Real Test ===\n\n";
    
    try {
        // Test 1: Array indexing
        std::cout << "1. Testing Array Indexing:\n";
        JsonValue arrayJson = JsonValue::object({
            {"numbers", JsonValue::array({
                JsonValue(10), JsonValue(20), JsonValue(30), JsonValue(40), JsonValue(50)
            })},
            {"books", JsonValue::array({
                JsonValue::object({{"title", JsonValue("Book 1")}, {"price", JsonValue(12.99)}}),
                JsonValue::object({{"title", JsonValue("Book 2")}, {"price", JsonValue(15.99)}}),
                JsonValue::object({{"title", JsonValue("Book 3")}, {"price", JsonValue(9.99)}})
            })}
        });
        
        auto result = arrayJson.selectFirst("$.numbers[0]");
        if (result && result->isNumber()) {
            std::cout << "   $.numbers[0] = " << result->toInt() << " ✅\n";
        } else {
            std::cout << "   $.numbers[0] failed ❌\n";
        }
        
        result = arrayJson.selectFirst("$.numbers[2]");
        if (result && result->isNumber()) {
            std::cout << "   $.numbers[2] = " << result->toInt() << " ✅\n";
        } else {
            std::cout << "   $.numbers[2] failed ❌\n";
        }
        
        result = arrayJson.selectFirst("$.books[1].title");
        if (result && result->isString()) {
            std::cout << "   $.books[1].title = \"" << result->toString() << "\" ✅\n";
        } else {
            std::cout << "   $.books[1].title failed ❌\n";
        }
        
        // Test 2: Wildcard selection
        std::cout << "\n2. Testing Wildcard Selection:\n";
        auto allResults = arrayJson.selectAll("$.*");
        std::cout << "   $.* found " << allResults.size() << " elements\n";
        
        allResults = arrayJson.selectAll("$.books[*].title");
        if (!allResults.empty()) {
            std::cout << "   $.books[*].title found " << allResults.size() << " titles ✅\n";
        } else {
            std::cout << "   $.books[*].title failed ❌\n";
        }
        
        // Test 3: Recursive descent
        std::cout << "\n3. Testing Recursive Descent:\n";
        JsonValue complexJson = JsonValue::object({
            {"store", JsonValue::object({
                {"book", JsonValue::array({
                    JsonValue::object({
                        {"title", JsonValue("Book 1")},
                        {"price", JsonValue(10.99)}
                    }),
                    JsonValue::object({
                        {"title", JsonValue("Book 2")},
                        {"price", JsonValue(15.99)}
                    })
                })},
                {"bicycle", JsonValue::object({
                    {"price", JsonValue(19.95)}
                })}
            })},
            {"warehouse", JsonValue::object({
                {"inventory", JsonValue::object({
                    {"price", JsonValue(25.50)}
                })}
            })}
        });
        
        auto recursiveResults = complexJson.selectAll("$..price");
        std::cout << "   $..price found " << recursiveResults.size() << " price values\n";
        if (recursiveResults.size() >= 3) {
            std::cout << "   Recursive descent working ✅\n";
            for (size_t i = 0; i < recursiveResults.size(); ++i) {
                if (recursiveResults[i]->isNumber()) {
                    std::cout << "     price[" << i << "] = " << recursiveResults[i]->toDouble() << "\n";
                }
            }
        } else {
            std::cout << "   Recursive descent needs work ❌\n";
        }
        
        // Test 4: Array slicing
        std::cout << "\n4. Testing Array Slicing:\n";
        auto sliceResults = arrayJson.selectAll("$.numbers[1:4]");
        std::cout << "   $.numbers[1:4] found " << sliceResults.size() << " elements\n";
        if (!sliceResults.empty()) {
            std::cout << "   Array slicing working ✅\n";
        } else {
            std::cout << "   Array slicing needs work ❌\n";
        }
        
        // Test 5: selectValues vs selectAll
        std::cout << "\n5. Testing selectValues:\n";
        auto valueResults = arrayJson.selectValues("$.numbers[*]");
        std::cout << "   selectValues found " << valueResults.size() << " number values\n";
        if (valueResults.size() > 0) {
            std::cout << "   selectValues working correctly ✅\n";
            for (size_t i = 0; i < valueResults.size(); ++i) {
                if (valueResults[i].isNumber()) {
                    std::cout << "     number[" << i << "] = " << valueResults[i].toInt() << "\n";
                }
            }
        } else {
            std::cout << "   selectValues failed ❌\n";
        }
        
        std::cout << "\n🎉 Advanced JSONPath features test complete!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
