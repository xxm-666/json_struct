#include <iostream>
#include "../src/json_engine/json_path.h"
#include "../src/json_engine/json_value.h"
#include "test_framework.h"

using namespace JsonStruct;
using namespace jsonvalue_jsonpath;
using namespace TestFramework;

TEST(SpacedPropertyNames) {
    // Create test JSON with spaced property names
    JsonValue root = JsonValue::object({
        {"x", JsonValue::array({
            JsonValue::object({
                {"test a", JsonValue("value1")},
                {"normal_prop", JsonValue("normal1")}
            }),
            JsonValue::object({
                {"test a", JsonValue("value2")},
                {"normal_prop", JsonValue("normal2")}
            }),
            JsonValue::object({
                {"test a", JsonValue("value3")},
                {"another prop", JsonValue("another_value")}
            })
        })}
    });
    // Test 1: Using bracket notation with quotes for spaced property name
    auto results1 = selectAll(root, "$.x[*]['test a']");
    ASSERT_TRUE(results1.size() == 3);
    ASSERT_TRUE(results1[0].get().toString() == "value1");
    // Test 2: Using bracket notation with double quotes
    auto results2 = selectAll(root, "$.x[*][\"test a\"]");
    ASSERT_TRUE(results2.size() == 3);
    ASSERT_TRUE(results2[1].get().toString() == "value2");
    // Test 3: Try direct dot notation (should fail)
    auto results3 = selectAll(root, "$.x[*].test a");

    ASSERT_TRUE(results3.empty());
    // Test 4: Another property with space
    auto results4 = selectAll(root, "$.x[*]['another prop']");
    ASSERT_TRUE(results4.size() == 1);
    ASSERT_TRUE(results4[0].get().toString() == "another_value");
    // Test 5: Mutable operations with spaced property names
    auto mutable_results = selectAllMutable(root, "$.x[*]['test a']");
    ASSERT_TRUE(mutable_results.size() == 3);
    for (auto& ref : mutable_results) {
        std::string old_value = ref.get().toString();
        ref.get() = JsonValue(old_value + "_modified");
    }
    auto verify_results = selectAll(root, "$.x[*]['test a']");
    ASSERT_TRUE(verify_results[0].get().toString() == "value1_modified");
}

TEST(ComplexSpacedPaths) {
    JsonValue root = JsonValue::object({
        {"data source", JsonValue::object({
            {"user info", JsonValue::array({
                JsonValue::object({
                    {"first name", JsonValue("John")},
                    {"last name", JsonValue("Doe")},
                    {"contact details", JsonValue::object({
                        {"home address", JsonValue("123 Main St")},
                        {"work phone", JsonValue("555-1234")}
                    })}
                }),
                JsonValue::object({
                    {"first name", JsonValue("Jane")},
                    {"last name", JsonValue("Smith")},
                    {"contact details", JsonValue::object({
                        {"home address", JsonValue("456 Oak Ave")},
                        {"work phone", JsonValue("555-5678")}
                    })}
                })
            })}
        })}
    });
    // Test deeply nested spaced property access
    auto results = selectAll(root, "$['data source']['user info'][*]['first name']");
    ASSERT_TRUE(results.size() == 2);
    ASSERT_TRUE(results[0].get().toString() == "John");
    ASSERT_TRUE(results[1].get().toString() == "Jane");
    // Test recursive with spaced properties
    auto phones = selectAll(root, "$..['work phone']");
    ASSERT_TRUE(phones.size() == 2);
    ASSERT_TRUE(phones[0].get().toString() == "555-1234");
    ASSERT_TRUE(phones[1].get().toString() == "555-5678");
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}
