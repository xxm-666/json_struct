#include <iostream>
#include <test_framework/test_framework.h>
#include "json_engine/json_filter.h"
#include "json_engine/json_value.h"
#include "json_engine/lazy_query_generator.h"
using namespace JsonStruct;
using namespace JsonStruct::literals;

TEST(JsonFilter_LazyQuery) {
    try {
        JsonFilter filter = JsonFilter::createDefault();
        
        auto json = R"(
            {
                "name": "test_string",
                "value": 42,
                "nested": {
                    "title": "nested_string",
                    "count": 100
                },
                "array": [
                    {"item": "array_item1"},
                    {"item": "array_item2"}
                ],
                "special": {
                    "key.with.dots": "value3",
                    "key/with/slashes": "value4"
                },
                "mixed_array": [
                    "string_item",
                    42,
                    3.14,
                    true,
                    null,
                    {"type": "embedded_object", "value": 100}
                ],
                "item_array": [
                    {"id": 0, "name": "Item_0", "category": "Category_0", "price": 0.0},
                    {"id": 1, "name": "Item_1", "category": "Category_1", "price": 0.99},
                    {"id": 2, "name": "Item_2", "category": "Category_2", "price": 1.98},
                    {"id": 3, "name": "Item_3", "category": "Category_3", "price": 2.97}
                ]
            }
        )"_json;
        JsonValue data(std::move(json));
        
        // Test basic filter
        FilterFunction stringFilter = [](const JsonValue& value, const std::string& path) -> bool {
            return value.isString();
        };
        
        std::cout << "Creating queryGenerator..." << std::endl;
        auto generator = filter.queryGenerator(data, stringFilter);
        
        std::cout << "Checking hasNext..." << std::endl;
        int count = 0;
        while (generator.hasNext()) {
            std::cout << "Getting next result..." << std::endl;
            auto result = generator.next();
            std::cout << "Found result at path: " << result.path << ", value: " << result.value->toString() << std::endl;
            count++;
        }
        
        if (count == 0) {
            std::cout << "No results found" << std::endl;
        } else {
            std::cout << "Found " << count << " string results" << std::endl;
        }
        
        ASSERT_EQ(16, count);
        std::cout << "Test completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

int main() {
  RUN_ALL_TESTS();
  return 0;
}