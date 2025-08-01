#include "jsonstruct.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <map>

using namespace JsonStruct;
using namespace JsonStruct::literals;

void demonstrateBasics() {
    std::cout << "=== Basic Features Demo ===" << std::endl;
    
    JsonValue null_val;
    JsonValue bool_val(true);
    JsonValue int_val(42);
    JsonValue str_val("Hello, World!");
    
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    JsonValue array_val(numbers);
    
    std::map<std::string, std::string> person = {
        {"name", "Alice"},
        {"city", "Beijing"}
    };
    JsonValue object_val(person);
    
    std::cout << "Array: " << array_val.dump() << std::endl;
    std::cout << "Object: " << object_val.dump() << std::endl;
}

void demonstrateModernSyntax() {
    std::cout << "\n=== Modern Syntax Demo ===" << std::endl;
    
    auto config = R"({
        "app": {
            "name": "MyApp",
            "version": "1.0"
        },
        "users": ["Alice", "Bob", "Charlie"],
        "settings": {
            "debug": true,
            "maxConnections": 100
        }
    })"_json;
    
    std::cout << "Original config:\n" << config.dump(2) << std::endl;
    
    auto new_user = makeJson("David");
    config["users"].append(new_user);
    
    std::cout << "\nApp name: " << config.at("/app/name").toString() << std::endl;
    std::cout << "Debug mode: " << (config.at("/settings/debug").toBool() ? "on" : "off") << std::endl;
    
    config.at("/settings/maxConnections") = makeJson(200);
    config.at("/app/version") = makeJson("1.1");
    
    std::cout << "\nUpdated config:\n" << config.dump(2) << std::endl;
}

void demonstrateTypeSafety() {
    std::cout << "\n=== Type Safety Demo ===" << std::endl;
    
    auto data = R"({
        "name": "Alice",
        "age": 30,
        "score": 95.5,
        "active": true,
        "tags": ["student", "programmer"]
    })"_json;
    
    if (auto name = data["name"].getString()) {
        std::cout << "Name: " << *name << std::endl;
    }
    
    if (auto age = data["age"].getNumber()) {
        std::cout << "Age: " << static_cast<int>(*age) << std::endl;
    }
    
    if (data["tags"].isArray()) {
        std::cout << "Tags: ";
        const auto& tags = data["tags"].toArray();
        for (size_t i = 0; i < tags.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << tags[i].toString();
        }
        std::cout << std::endl;
    }
    
    if (auto missing = data["missing_key"].getString()) {
        std::cout << "Should not reach here" << std::endl;
    } else {
        std::cout << "Missing key correctly returns nullopt" << std::endl;
    }
}

void demonstrateVisitorPattern() {
    std::cout << "\n=== Visitor Pattern Demo ===" << std::endl;
    
    auto mixed_array = R"([42, "hello", true, null, [1,2,3]])"_json;
    
    std::cout << "Array content analysis:" << std::endl;
    for (size_t i = 0; i < mixed_array.size(); ++i) {
        std::cout << "Index " << i << ": ";
        
        mixed_array[i].visit([](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            
            if constexpr (std::is_same_v<T, std::monostate>) {
                std::cout << "null value";
            } else if constexpr (std::is_same_v<T, bool>) {
                std::cout << "boolean: " << (value ? "true" : "false");
            } else if constexpr (std::is_same_v<T, double>) {
                std::cout << "number: " << value;
            } else if constexpr (std::is_same_v<T, std::string>) {
                std::cout << "string: \"" << value << "\"";
            } else if constexpr (std::is_same_v<T, JsonValue::ArrayType>) {
                std::cout << "array with " << value.size() << " elements";
            } else if constexpr (std::is_same_v<T, JsonValue::ObjectType>) {
                std::cout << "object with " << value.size() << " keys";
            }
        });
        
        std::cout << std::endl;
    }
}

void demonstratePerformance() {
    std::cout << "\n=== Performance Demo ===" << std::endl;
    
    auto createLargeObject = []() {
        JsonValue obj;
        for (int i = 0; i < 1000; ++i) {
            obj["key_" + std::to_string(i)] = makeJson("value_" + std::to_string(i));
        }
        return obj;
    };
    
    auto large_obj = createLargeObject();
    std::cout << "Created large object with " << large_obj.size() << " key-value pairs" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i) {
        auto value = large_obj["key_" + std::to_string(i)];
        (void)value;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "100 lookups took: " << duration.count() << " microseconds" << std::endl;
}

void demonstrateErrorHandling() {
    std::cout << "\n=== Error Handling Demo ===" << std::endl;
    
    std::string invalid_json = R"({
        "name": "test",
        "age": ,
        "city": "Beijing"
    })";
    
    try {
        auto parsed = JsonValue::parse(invalid_json);
    } catch (const std::exception& e) {
        std::cout << "Parse error: " << e.what() << std::endl;
    }
    
    auto valid_data = R"({"number": 42})"_json;
    
    try {
        auto invalid_access = valid_data.at("/number/invalid");
    } catch (const std::exception& e) {
        std::cout << "Access error: " << e.what() << std::endl;
    }
    
    JsonValue::ParseOptions depth_options;
    depth_options.maxDepth = 3;
    
    std::string deep_json = "[[[[42]]]]";
    
    try {
        auto parsed = JsonValue::parse(deep_json, depth_options);
    } catch (const std::exception& e) {
        std::cout << "Depth limit: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "JsonValue Feature Demonstration" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    try {
        demonstrateBasics();
        demonstrateModernSyntax();
        demonstrateTypeSafety();
        demonstrateVisitorPattern();
        demonstratePerformance();
        demonstrateErrorHandling();
        
        std::cout << "\nDemo completed successfully!" << std::endl;
        std::cout << "\nFor more details, please refer to README.md and docs/ADVANCED_FEATURES.md" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Demo failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
