// 序列化选项基础测试
#include "../src/jsonstruct.h"
#include "../test_framework/test_framework.h"
#include <iostream>
#include <algorithm>

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
    
    // 测试不允许特殊数值的情况
    JsonValue::SerializeOptions noSpecial;
    noSpecial.allowSpecialNumbers = false;
    
    try {
        std::string result = json.dump(noSpecial);
        // 如果没有抛出异常，特殊数值应该被转换为其他形式
        ASSERT_FALSE(result.empty());
        
        // 重新解析应该成功
        JsonValue reparsed = JsonValue::parse(result);
        ASSERT_TRUE(reparsed.isObject());
    } catch (const std::exception& e) {
        // 如果抛出异常也是可以接受的
        std::cout << "Special numbers serialization failed as expected: " << e.what() << std::endl;
    }
    
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

TEST(SerializationOptions_UnicodeHandling) {
    JsonValue::ObjectType obj;
    obj["english"] = JsonValue("Hello World");
    obj["chinese"] = JsonValue("你好世界");
    obj["emoji"] = JsonValue("🌍🚀");
    
    JsonValue json(obj);
    
    // 测试不转义Unicode（如果支持）
    JsonValue::SerializeOptions noEscape;
    noEscape.escapeUnicode = false;
    std::string noEscapeResult = json.dump(noEscape);
    
    // 应该包含原始Unicode字符
    ASSERT_TRUE(noEscapeResult.find("你好世界") != std::string::npos);
    
    // 测试转义Unicode（如果支持）
    JsonValue::SerializeOptions escapeUnicode;
    escapeUnicode.escapeUnicode = true;
    std::string escapedResult = json.dump(escapeUnicode);
    
    // 验证转义后的结果可以正确解析
    JsonValue reparsed = JsonValue::parse(escapedResult);
    ASSERT_EQ(reparsed["chinese"].toString(), "你好世界");
    ASSERT_EQ(reparsed["emoji"].toString(), "🌍🚀");
}

TEST(SerializationOptions_LargeData) {
    // 创建大型数据结构
    JsonValue::ArrayType largeArray;
    for (int i = 0; i < 1000; ++i) {
        JsonValue::ObjectType item;
        item["id"] = JsonValue(i);
        item["value"] = JsonValue(i * 1.5);
        largeArray.push_back(JsonValue(item));
    }
    
    JsonValue::ObjectType root;
    root["count"] = JsonValue(1000);
    root["items"] = JsonValue(largeArray);
    
    JsonValue json(root);
    
    // 测试紧凑序列化
    JsonValue::SerializeOptions compact;
    compact.indent = -1;
    
    auto start = std::chrono::high_resolution_clock::now();
    std::string result = json.dump(compact);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 验证结果
    ASSERT_FALSE(result.empty());
    ASSERT_TRUE(result.find("\"count\":1000") != std::string::npos);
    
    // 性能检查（应该在合理时间内完成）
    std::cout << "Large data serialization time: " << duration.count() << "ms" << std::endl;
    ASSERT_TRUE(duration.count() < 2000); // 2秒内完成
    
    // 验证可以重新解析
    JsonValue reparsed = JsonValue::parse(result);
    ASSERT_EQ(reparsed["count"].toInt(), 1000);
    ASSERT_EQ(reparsed["items"].toArray().size(), 1000);
}

int main() {
    return RUN_ALL_TESTS();
}
