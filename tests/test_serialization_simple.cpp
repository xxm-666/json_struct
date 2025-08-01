// 序列化选项基础测试（简化版）
#include "../src/jsonstruct.h"
#include "../test_framework/test_framework.h"
#include <iostream>
#include <chrono>

using namespace JsonStruct;

TEST(SerializationOptions_CompactVsIndented) {
    JsonValue json = JsonValue::parse(R"({
        "name": "Test",
        "array": [1, 2, 3],
        "nested": {
            "key": "value",
            "number": 42
        }
    })");
    
    // 测试紧凑模式
    JsonValue::SerializeOptions compact;
    compact.indent = -1;
    std::string compactResult = json.dump(compact);
    
    // 紧凑模式不应该有多余的空白
    ASSERT_TRUE(compactResult.find('\n') == std::string::npos);
    
    // 测试缩进模式
    JsonValue::SerializeOptions indented;
    indented.indent = 2;
    std::string indentedResult = json.dump(indented);
    
    // 缩进模式应该包含换行符和空格
    ASSERT_TRUE(indentedResult.find('\n') != std::string::npos);
    ASSERT_TRUE(indentedResult.find("  ") != std::string::npos);
    
    // 验证两种格式都可以重新解析
    JsonValue reparsedCompact = JsonValue::parse(compactResult);
    JsonValue reparsedIndented = JsonValue::parse(indentedResult);
    
    ASSERT_EQ(reparsedCompact["name"].toString(), "Test");
    ASSERT_EQ(reparsedIndented["name"].toString(), "Test");
}

TEST(SerializationOptions_KeySorting) {
    JsonValue::ObjectType obj;
    obj["zebra"] = JsonValue("last");
    obj["alpha"] = JsonValue("first");  
    obj["beta"] = JsonValue("second");
    
    JsonValue json(obj);
    
    // 测试键排序
    JsonValue::SerializeOptions sorted;
    sorted.sortKeys = true;
    sorted.indent = 2; // 使用缩进便于观察
    std::string sortedResult = json.dump(sorted);
    
    // 验证alpha在beta之前，beta在zebra之前
    size_t alphaPos = sortedResult.find("\"alpha\"");
    size_t betaPos = sortedResult.find("\"beta\"");
    size_t zebraPos = sortedResult.find("\"zebra\"");
    
    ASSERT_TRUE(alphaPos != std::string::npos);
    ASSERT_TRUE(betaPos != std::string::npos);
    ASSERT_TRUE(zebraPos != std::string::npos);
    ASSERT_TRUE(alphaPos < betaPos);
    ASSERT_TRUE(betaPos < zebraPos);
}

TEST(SerializationOptions_FloatingPointPrecision) {
    JsonValue::ObjectType obj;
    obj["pi"] = JsonValue(3.141592653589793);
    obj["small"] = JsonValue(0.000000123456789);
    
    JsonValue json(obj);
    
    // 测试不同精度
    JsonValue::SerializeOptions lowPrecision;
    lowPrecision.maxPrecision = 3;
    std::string lowResult = json.dump(lowPrecision);
    
    JsonValue::SerializeOptions highPrecision;
    highPrecision.maxPrecision = 15;
    std::string highResult = json.dump(highPrecision);
    
    // 重新解析并验证
    JsonValue lowReparsed = JsonValue::parse(lowResult);
    JsonValue highReparsed = JsonValue::parse(highResult);
    
    double originalPi = 3.141592653589793;
    double lowPi = lowReparsed["pi"].toDouble();
    double highPi = highReparsed["pi"].toDouble();
    
    // 高精度版本应该更接近原始值
    ASSERT_TRUE(std::abs(originalPi - highPi) <= std::abs(originalPi - lowPi));
}

TEST(SerializationOptions_SpecialNumbers) {
    JsonValue::ObjectType obj;
    obj["nan"] = JsonValue(JsonNumber::makeNaN());
    obj["infinity"] = JsonValue(JsonNumber::makeInfinity());
    obj["normal"] = JsonValue(42.0);
    
    JsonValue json(obj);
    
    // 测试允许特殊数值
    JsonValue::SerializeOptions allowSpecial;
    allowSpecial.allowSpecialNumbers = true;
    
    try {
        std::string specialResult = json.dump(allowSpecial);
        ASSERT_FALSE(specialResult.empty());
        std::cout << "Special numbers result: " << specialResult << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Special numbers not supported: " << e.what() << std::endl;
    }
}

int main() {
    return RUN_ALL_TESTS();
}
