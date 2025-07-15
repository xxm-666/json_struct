// 测试getString行为一致性
#include "../src/json_engine/json_value.h"
#include <iostream>

using namespace JsonStruct;

int main() {
    std::cout << "=== getString Consistency Test ===" << std::endl;
    
    std::string test_str = "hello";
    JsonValue json_val(test_str);
    
    std::cout << "1. JsonValue created with: '" << test_str << "'" << std::endl;
    
    // 多次调用getString
    for (int i = 0; i < 5; i++) {
        auto result = json_val.getString();
        std::cout << "   getString() call " << (i+1) << ": ";
        if (result) {
            std::cout << "SUCCESS - '" << *result << "'" << std::endl;
        } else {
            std::cout << "FAILED" << std::endl;
        }
    }
    
    // 测试toString的不同调用
    std::cout << "2. toString behavior:" << std::endl;
    std::cout << "   toString(): '" << json_val.toString() << "'" << std::endl;
    std::cout << "   toString(\"\"):  '" << json_val.toString("") << "'" << std::endl;
    std::cout << "   toString(\"DEFAULT\"): '" << json_val.toString("DEFAULT") << "'" << std::endl;
    
    // 检查getString是否仍然工作
    std::cout << "3. getString after toString:" << std::endl;
    auto final_result = json_val.getString();
    if (final_result) {
        std::cout << "   SUCCESS - '" << *final_result << "'" << std::endl;
    } else {
        std::cout << "   FAILED" << std::endl;
    }
    
    return 0;
}
