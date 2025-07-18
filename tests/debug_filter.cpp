#include <iostream>
#include "../src/json_engine/json_path.h"
#include "../src/json_engine/json_value.h"

using namespace JsonStruct;
using namespace jsonvalue_jsonpath;

int main() {
    try {
        std::cout << "Debug Filter Test\n" << std::endl;
        
        JsonValue root = JsonValue::object({
            {"users", JsonValue::array({
                JsonValue::object({
                    {"name", JsonValue("Alice")},
                    {"age", JsonValue(85)},
                    {"user name", JsonValue("Alice Space")},
                    {"age score", JsonValue(85)}
                }),
                JsonValue::object({
                    {"name", JsonValue("Bob")},
                    {"age", JsonValue(95)},
                    {"user name", JsonValue("Bob Space")},
                    {"age score", JsonValue(95)}
                })
            })}
        });
        
        std::cout << "Test data: " << root.dump(2) << std::endl;
        
        // Test basic filter without spaces
        std::cout << "\n--- Test 1: $.users[?(@.age > 80)] ---" << std::endl;
        try {
            auto results = selectAll(root, "$.users[?(@.age > 80)]");
            std::cout << "Found " << results.size() << " matches" << std::endl;
            for (const auto& result : results) {
                std::cout << "  " << result.get()["name"].dump() << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        
        // Test filter with spaces using bracket notation
        std::cout << "\n--- Test 2: $.users[?(@['age score'] > 80)] ---" << std::endl;
        try {
            auto results = selectAll(root, "$.users[?(@['age score'] > 80)]");
            std::cout << "Found " << results.size() << " matches" << std::endl;
            for (const auto& result : results) {
                std::cout << "  " << result.get()["user name"].dump() << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        
        // Test simpler condition
        std::cout << "\n--- Test 3: $.users[?(@['age score'] == 95)] ---" << std::endl;
        try {
            auto results = selectAll(root, "$.users[?(@['age score'] == 95)]");
            std::cout << "Found " << results.size() << " matches" << std::endl;
            for (const auto& result : results) {
                std::cout << "  " << result.get()["user name"].dump() << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
