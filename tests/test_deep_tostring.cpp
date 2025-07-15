// 更深入的toString调试
#include "../src/json_engine/json_value.h"
#include <iostream>

using namespace JsonStruct;

int main() {
    std::cout << "=== Deep toString Debug ===" << std::endl;
    
    std::string test_str = "hello";
    JsonValue json_val(test_str);
    
    std::cout << "1. Basic info:" << std::endl;
    std::cout << "   isString(): " << json_val.isString() << std::endl;
    std::cout << "   type(): " << static_cast<int>(json_val.type()) << std::endl;
    
    std::cout << "2. getString behavior:" << std::endl;
    auto result1 = json_val.getString();
    std::cout << "   result1 has_value: " << result1.has_value() << std::endl;
    if (result1) {
        std::cout << "   result1 value: '" << *result1 << "'" << std::endl;
    }
    
    std::cout << "3. toString() - no args (should use defaultValue=\"\"):" << std::endl;
    std::string toString_no_args = json_val.toString();
    std::cout << "   result: '" << toString_no_args << "'" << std::endl;
    std::cout << "   length: " << toString_no_args.length() << std::endl;
    std::cout << "   equals original: " << (toString_no_args == test_str) << std::endl;
    
    std::cout << "4. toString(\"\") - explicit empty default:" << std::endl;
    std::string toString_empty = json_val.toString("");
    std::cout << "   result: '" << toString_empty << "'" << std::endl;
    std::cout << "   length: " << toString_empty.length() << std::endl;
    std::cout << "   equals original: " << (toString_empty == test_str) << std::endl;
    
    std::cout << "5. toString(\"X\") - explicit non-empty default:" << std::endl;
    std::string toString_x = json_val.toString("X");
    std::cout << "   result: '" << toString_x << "'" << std::endl;
    std::cout << "   length: " << toString_x.length() << std::endl;
    std::cout << "   equals original: " << (toString_x == test_str) << std::endl;
    
    return 0;
}
