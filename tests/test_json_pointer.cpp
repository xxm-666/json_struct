#include "json_engine/json_value.h"
#include <cassert>
#include <iostream>

using namespace JsonStruct;

int main() {
    // Create a nested JSON object with arrays and edge cases
    JsonValue json = JsonValue::parse(R"({
        "a": {
            "b": {
                "c": {
                    "d": 42,
                    "arr": [1, 2, 3],
                    "empty": {},
                    "nullval": null
                }
            }
        },
        "rootval": "hello"
    })");

    // 1. Valid deep pointer
    try {
        const JsonValue& val = json.at("/a/b/c/d");
        assert(val.isNumber());
        assert(val.getNumber().value_or(0) == 42);
        std::cout << "JSON Pointer /a/b/c/d access test passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "JSON Pointer /a/b/c/d test failed: " << e.what() << std::endl;
        return 1;
    }

    // 2. Root pointer
    try {
        const JsonValue& val = json.at("");
        assert(val.isObject());
        std::cout << "JSON Pointer '' (root) access test passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "JSON Pointer root test failed: " << e.what() << std::endl;
        return 1;
    }

    // 3. Top-level property
    try {
        const JsonValue& val = json.at("/rootval");
        assert(val.isString());
        assert(val.getString().value_or("") == "hello");
        std::cout << "JSON Pointer /rootval access test passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "JSON Pointer /rootval test failed: " << e.what() << std::endl;
        return 1;
    }

    // 4. Array index
    try {
        const JsonValue& val = json.at("/a/b/c/arr/1");
        assert(val.isNumber());
        assert(val.getNumber().value_or(0) == 2);
        std::cout << "JSON Pointer /a/b/c/arr/1 access test passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "JSON Pointer /a/b/c/arr/1 test failed: " << e.what() << std::endl;
        return 1;
    }

    // 5. Empty object
    try {
        const JsonValue& val = json.at("/a/b/c/empty");
        assert(val.isObject());
        assert(val.size() == 0);
        std::cout << "JSON Pointer /a/b/c/empty access test passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "JSON Pointer /a/b/c/empty test failed: " << e.what() << std::endl;
        return 1;
    }

    // 6. Null value
    try {
        const JsonValue& val = json.at("/a/b/c/nullval");
        assert(val.isNull());
        std::cout << "JSON Pointer /a/b/c/nullval access test passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "JSON Pointer /a/b/c/nullval test failed: " << e.what() << std::endl;
        return 1;
    }

    // 7. Out-of-bounds array index
    try {
        json.at("/a/b/c/arr/10");
        std::cerr << "JSON Pointer /a/b/c/arr/10 should have thrown!" << std::endl;
        return 1;
    } catch (const std::exception&) {
        std::cout << "JSON Pointer /a/b/c/arr/10 out-of-bounds test passed!" << std::endl;
    }

    // 8. Non-existent property
    try {
        json.at("/a/b/c/notfound");
        std::cerr << "JSON Pointer /a/b/c/notfound should have thrown!" << std::endl;
        return 1;
    } catch (const std::exception&) {
        std::cout << "JSON Pointer /a/b/c/notfound not-found test passed!" << std::endl;
    }

    // 9. Type error (try to index into non-container)
    try {
        json.at("/a/b/c/d/0");
        std::cerr << "JSON Pointer /a/b/c/d/0 type error should have thrown!" << std::endl;
        return 1;
    } catch (const std::exception&) {
        std::cout << "JSON Pointer /a/b/c/d/0 type error test passed!" << std::endl;
    }

    std::cout << "All JSON Pointer edge tests passed!" << std::endl;
    return 0;
}
