#include "json_engine/json_patch.h"
#include "../test_framework/test_framework.h"
#include <iostream>

using namespace JsonStruct::literals;

TEST(RFC7396_BasicMergePatch) {
    // Test case from RFC 7396
    JsonStruct::JsonValue target = R"({
        "a": "b",
        "c": {
            "d": "e",
            "f": "g"
        }
    })"_json;
    
    JsonStruct::JsonValue patch = R"({
        "a": "z",
        "c": {
            "f": null
        }
    })"_json;
    
    JsonStruct::JsonPatch::ApplyPatch(target, patch);
    
    ASSERT_EQ(target["a"].toString(), "z");
    ASSERT_EQ(target["c"]["d"].toString(), "e");
    ASSERT_FALSE(target["c"].contains("f"));
    ASSERT_EQ(target["c"].size(), 1);
}

TEST(RFC7396_AddingNewKeys) {
    JsonStruct::JsonValue target = R"({"a":"b"})"_json;
    JsonStruct::JsonValue patch = R"({"c":"d"})"_json;
    
    JsonStruct::JsonPatch::ApplyPatch(target, patch);
    
    ASSERT_EQ(target["a"].toString(), "b");
    ASSERT_EQ(target["c"].toString(), "d");
    ASSERT_EQ(target.size(), 2);
}

TEST(RFC7396_RemovingKeysWithNull) {
    JsonStruct::JsonValue target = R"({"a":"b","c":"d"})"_json;
    JsonStruct::JsonValue patch = R"({"a":null})"_json;
    
    JsonStruct::JsonPatch::ApplyPatch(target, patch);
    
    ASSERT_FALSE(target.contains("a"));
    ASSERT_EQ(target["c"].toString(), "d");
    ASSERT_EQ(target.size(), 1);
}

TEST(RFC7396_NestedObjects) {
    JsonStruct::JsonValue target = R"({"a":{"b":"c"}})"_json;
    JsonStruct::JsonValue patch = R"({"a":{"b":"d","c":null}})"_json;
    
    JsonStruct::JsonPatch::ApplyPatch(target, patch);
    
    ASSERT_EQ(target["a"]["b"].toString(), "d");
    ASSERT_FALSE(target["a"].contains("c"));
    ASSERT_EQ(target["a"].size(), 1);
}

TEST(RFC7396_ReplaceEntireDocument) {
    JsonStruct::JsonValue target = R"({"a":"b"})"_json;
    JsonStruct::JsonValue patch = R"({"a":null,"c":"d"})"_json;
    
    JsonStruct::JsonPatch::ApplyPatch(target, patch);
    
    ASSERT_FALSE(target.contains("a"));
    ASSERT_EQ(target["c"].toString(), "d");
    ASSERT_EQ(target.size(), 1);
}

TEST(RFC7396_NullPatch) {
    JsonStruct::JsonValue target = R"({"a":"b"})"_json;
    JsonStruct::JsonValue patch; // null value
    
    JsonStruct::JsonPatch::ApplyPatch(target, patch);
    
    ASSERT_TRUE(target.isNull());
}

TEST(RFC7396_NonObjectTarget) {
    JsonStruct::JsonValue target = R"("string_value")"_json;
    JsonStruct::JsonValue patch = R"({"a":"b"})"_json;
    
    JsonStruct::JsonPatch::ApplyPatch(target, patch);
    
    ASSERT_EQ(target["a"].toString(), "b");
    ASSERT_EQ(target.size(), 1);
}

TEST(RFC7396_NonObjectPatch) {
    JsonStruct::JsonValue target = R"({"a":"b"})"_json;
    JsonStruct::JsonValue patch = R"("string_value")"_json;
    
    JsonStruct::JsonPatch::ApplyPatch(target, patch);
    
    ASSERT_EQ(target.toString(), "string_value");
}

TEST(RFC7396_EmptyPatch) {
    JsonStruct::JsonValue target = R"({"a":"b","c":"d"})"_json;
    JsonStruct::JsonValue patch = R"({})"_json;
    
    JsonStruct::JsonPatch::ApplyPatch(target, patch);
    
    ASSERT_EQ(target["a"].toString(), "b");
    ASSERT_EQ(target["c"].toString(), "d");
    ASSERT_EQ(target.size(), 2);
}

TEST(RFC7396_ComplexNestedStructure) {
    JsonStruct::JsonValue target = R"({
        "a": {
            "b": {
                "c": "d"
            },
            "e": [1, 2, 3]
        },
        "f": "g"
    })"_json;
    
    JsonStruct::JsonValue patch = R"({
        "a": {
            "b": {
                "c": "x",
                "y": "z"
            },
            "e": null
        }
    })"_json;
    
    JsonStruct::JsonPatch::ApplyPatch(target, patch);
    
    ASSERT_EQ(target["a"]["b"]["c"].toString(), "x");
    ASSERT_EQ(target["a"]["b"]["y"].toString(), "z");
    ASSERT_FALSE(target["a"].contains("e"));
    ASSERT_EQ(target["f"].toString(), "g");
}

TEST(RFC7396_ArrayHandling) {
    JsonStruct::JsonValue target = R"({
        "a": [1, 2, 3],
        "b": "c"
    })"_json;
    
    JsonStruct::JsonValue patch = R"({
        "a": [4, 5],
        "b": null
    })"_json;
    
    JsonStruct::JsonPatch::ApplyPatch(target, patch);
    
    ASSERT_EQ(target["a"].size(), 2);
    ASSERT_EQ(target["a"][0].toInt(), 4);
    ASSERT_EQ(target["a"][1].toInt(), 5);
    ASSERT_FALSE(target.contains("b"));
}

TEST(SpecialPaths_JSONPointer) {
    JsonStruct::JsonValue target = R"({
        "a": {
            "b": "c"
        },
        "d": [1, 2, 3]
    })"_json;
    
    // Test JSON Pointer path
    JsonStruct::JsonPatch::ApplyPatch(target, "/a/b", JsonStruct::JsonValue("modified"));
    ASSERT_EQ(target["a"]["b"].toString(), "modified");
    
    // Test JSON Pointer array access
    JsonStruct::JsonPatch::ApplyPatch(target, "/d/1", JsonStruct::JsonValue(42));
    ASSERT_EQ(target["d"][1].toInt(), 42);
}

TEST(SpecialPaths_JSONPath) {
    JsonStruct::JsonValue target = R"({
        "a": {
            "b": "c"
        },
        "d": [1, 2, 3]
    })"_json;
    
    // Test JSONPath
    JsonStruct::JsonPatch::ApplyPatch(target, "$.a.b", JsonStruct::JsonValue("modified"));
    ASSERT_EQ(target["a"]["b"].toString(), "modified");
    
    // Test JSONPath array access
    JsonStruct::JsonPatch::ApplyPatch(target, "$.d[1]", JsonStruct::JsonValue(42));
    ASSERT_EQ(target["d"][1].toInt(), 42);
}

TEST(SpecialPaths_EscapedCharacters) {
    JsonStruct::JsonValue target = R"({
        "a/b": "value1",
        "c": {
            "d~e": "value2"
        }
    })"_json;
    
    // Test escaped characters in JSON Pointer
    JsonStruct::JsonPatch::ApplyPatch(target, "/a~1b", JsonStruct::JsonValue("modified1"));
    ASSERT_EQ(target["a/b"].toString(), "modified1");
    
    JsonStruct::JsonPatch::ApplyPatch(target, "/c/d~0e", JsonStruct::JsonValue("modified2"));
    ASSERT_EQ(target["c"]["d~e"].toString(), "modified2");
}

TEST(SpecialPaths_RootPath) {
    JsonStruct::JsonValue target = R"({"a":"b"})"_json;
    
    // Test root path replacement
    JsonStruct::JsonValue newValue = R"({"c":"d"})"_json;
    JsonStruct::JsonPatch::ApplyPatch(target, "/", newValue);
    ASSERT_EQ(target["c"].toString(), "d");
    ASSERT_FALSE(target.contains("a"));
}

TEST(SpecialPaths_NullValue) {
    JsonStruct::JsonValue target = R"({
        "a": "b",
        "c": "d"
    })"_json;
    
    // Test removing a value by setting it to null
    JsonStruct::JsonPatch::ApplyPatch(target, "/a", JsonStruct::JsonValue());
    ASSERT_FALSE(target.contains("a"));
    ASSERT_EQ(target["c"].toString(), "d");
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}
