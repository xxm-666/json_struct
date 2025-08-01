// 最简单的JsonValue测试
#include "../src/json_engine/json_value.h"
#include <iostream>

using namespace JsonStruct;

int main() {
    std::cout << "=== Minimal JsonValue Test ===" << std::endl;
    
    std::string test_str = "hello";
    JsonValue json_val(test_str);
    
    std::cout << "1. Created JsonValue with: '" << test_str << "'" << std::endl;
    std::cout << "2. isString(): " << json_val.isString() << std::endl;
    
    auto get_result = json_val.getString();
    std::cout << "3. getString() result: ";
    if (get_result) {
        std::cout << "SUCCESS, value='" << *get_result << "'" << std::endl;
    } else {
        std::cout << "FAILED" << std::endl;
    }
    
    std::cout << "4. toString() test:" << std::endl;
    auto to_string = json_val.toString();
    std::cout << "   internal getString(): ";
    if (!to_string.empty()) {
        std::cout << "SUCCESS" << std::endl;
        // std::string result = std::string(*internal_get);
        std::cout << "   converted to string: '" << to_string << "'" << std::endl;
    } else {
        std::cout << "FAILED" << std::endl;
    }
    
    std::string final_result = json_val.toString("FALLBACK");
    std::cout << "5. Final toString(): '" << final_result << "'" << std::endl;
    
    int a = 5;
    JsonValue not_string(a);
    std::cout << "6. Created JsonValue with int: " << a << std::endl;
    auto not_string_v = not_string.toString("not a string");
    std::cout << "non-string JsonValue: '" << not_string_v << "'" << std::endl;

    return 0;
}
