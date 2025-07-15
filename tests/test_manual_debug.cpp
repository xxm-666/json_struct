// 手动调试toString逻辑
#include "../src/json_engine/json_value.h"
#include <iostream>

using namespace JsonStruct;

int main() {
    std::cout << "=== Manual toString Debug ===" << std::endl;
    
    std::string test_str = "hello";
    JsonValue json_val(test_str);
    
    std::cout << "1. JsonValue created with: '" << test_str << "'" << std::endl;
    
    // 调用实际的toString几次
    std::cout << "2. Multiple toString() calls:" << std::endl;
    std::string result1 = json_val.toString("FALLBACK1");
    std::cout << "   first call: '" << result1 << "'" << std::endl;
    
    std::string result2 = json_val.toString("FALLBACK2");
    std::cout << "   second call: '" << result2 << "'" << std::endl;
    
    std::string result3 = json_val.toString("");
    std::cout << "   third call (empty default): '" << result3 << "'" << std::endl;
    
    // 检查类型
    std::cout << "3. Type information:" << std::endl;
    std::cout << "   isString(): " << json_val.isString() << std::endl;
    std::cout << "   type(): " << static_cast<int>(json_val.type()) << std::endl;
    
    // 测试不同的构造方式
    std::cout << "4. Different construction methods:" << std::endl;
    
    // 使用const char*
    JsonValue json_val2("hello");
    std::cout << "   const char* construction toString(): '" << json_val2.toString("FAIL") << "'" << std::endl;
    
    // 使用string copy
    std::string str_copy = test_str;
    JsonValue json_val3(str_copy);
    std::cout << "   string copy construction toString(): '" << json_val3.toString("FAIL") << "'" << std::endl;
    
    // 使用move
    std::string str_move = test_str;
    JsonValue json_val4(std::move(str_move));
    std::cout << "   string move construction toString(): '" << json_val4.toString("FAIL") << "'" << std::endl;
    
    return 0;
}
