// 直接调试JsonValue内部variant
#include "../src/json_engine/json_value.h"
#include <iostream>
#include <variant>

using namespace JsonStruct;

int main() {
    std::cout << "=== Variant Debug ===" << std::endl;
    
    std::string test_str = "hello";
    JsonValue json_val(test_str);
    
    std::cout << "1. JsonValue created with: '" << test_str << "'" << std::endl;
    std::cout << "2. JsonValue.isString(): " << json_val.isString() << std::endl;
    
    // 尝试直接访问内部variant（如果可能的话）
    // 这需要访问私有成员，可能不行，但我们试试
    
    // 或者使用dump来看JSON输出
    std::cout << "3. JSON dump:" << std::endl;
    try {
        std::string json_str = json_val.dump();
        std::cout << "   dump result: '" << json_str << "'" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "   dump failed: " << e.what() << std::endl;
    }
    
    // 测试toString的各种变体
    std::cout << "4. toString variants:" << std::endl;
    std::cout << "   toString(): '" << json_val.toString() << "'" << std::endl;
    std::cout << "   toString(\"\"):  '" << json_val.toString("") << "'" << std::endl;
    std::cout << "   toString(\"X\"): '" << json_val.toString("X") << "'" << std::endl;
    
    // 比较with original string
    std::cout << "5. Comparison:" << std::endl;
    std::cout << "   original: '" << test_str << "'" << std::endl;
    std::cout << "   toString == original: " << (json_val.toString() == test_str) << std::endl;
    
    return 0;
}
