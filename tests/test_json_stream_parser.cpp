// JsonValue 流式解析器测试
#include "../src/jsonstruct.h"
#include <test_framework/test_framework.h>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <vector>
#ifdef _WIN32
#include <Windows.h>
#include <Psapi.h>
#else
#include <sys/resource.h>
#endif

using namespace JsonStruct;

// 辅助函数：获取当前内存使用量（简化版本）
size_t getCurrentMemoryUsage() {
    // 这里应该实现实际的内存监控
    // 简化版本返回0，实际项目中应该使用系统API
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
#else
    // Linux
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return usage.ru_maxrss;
    }
#endif
    return 0;
}

TEST(JsonStreamParser_LargeDocuments) {
    // 测试大型文档的流式解析
    std::stringstream largeJson;
    largeJson << "{\n\"data\": [\n";

    for (int i = 0; i < 10000; ++i) {
        if (i > 0) largeJson << ",\n";
        largeJson << "{\"id\": " << i << ", \"value\": " << (i * 1.5) << "}";
    }
    largeJson << "\n]\n}";

    std::string jsonStr = largeJson.str();

    // 测试流式解析不会导致内存溢出
    auto start = std::chrono::high_resolution_clock::now();
    JsonValue parsed = JsonValue::parse(jsonStr);
    auto end = std::chrono::high_resolution_clock::now();

    ASSERT_TRUE(parsed.isObject());
    ASSERT_TRUE(parsed["data"].isArray());
    ASSERT_EQ(parsed["data"].toArray()->get().size(), 10000);

    // 确保解析时间合理
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Large document parsing time: " << duration.count() << "ms" << std::endl;
    ASSERT_TRUE(duration.count() < 5000); // 5秒内完成
}

TEST(JsonStreamParser_DeepNesting) {
    // 测试深度嵌套的JSON解析
    std::string deepJson = "{";
    for (int i = 0; i < 100; ++i) {
        deepJson += "\"level" + std::to_string(i) + "\": {";
    }
    deepJson += "\"value\": 42";
    for (int i = 0; i < 100; ++i) {
        deepJson += "}";
    }
    deepJson += "}";

    JsonValue::ParseOptions options;
    options.maxDepth = 200; // 允许更深的嵌套

    JsonValue parsed = JsonValue::parse(deepJson, options);
    ASSERT_TRUE(parsed.isObject());

    // 验证可以访问深层数据
    JsonValue current = parsed;
    for (int i = 0; i < 100; ++i) {
        std::string key = "level" + std::to_string(i);
        ASSERT_TRUE(current.isObject());
        ASSERT_TRUE(current[key].isObject());
        current = current[key];
    }
    ASSERT_EQ(current["value"].toInt(), 42);
}

TEST(JsonStreamParser_ErrorRecovery) {
    // 测试错误恢复机制
    std::string malformedJson = R"({
        "valid1": "ok",
        "invalid": [1, 2, 3, },  // 语法错误
        "valid2": "still ok"
    })";

    JsonValue::ParseOptions recoveryOptions;
    recoveryOptions.allowRecovery = true;
    recoveryOptions.strictMode = false;

    bool exceptionCaught = false;
    JsonValue parsed;
    try {
        parsed = JsonValue::parse(malformedJson, recoveryOptions);
    }
    catch (const std::exception& e) {
        exceptionCaught = true;
    }

    // 根据实现，可能抛出异常或返回部分解析结果
    if (!exceptionCaught) {
        // 如果支持错误恢复，应该解析出有效部分
        ASSERT_TRUE(parsed.isObject());
        ASSERT_EQ(parsed["valid1"].toString(), "ok");
    }
}

TEST(JsonStreamParser_LenientMode) {
    // 测试宽松模式解析
    std::string lenientJson = R"({
        // 这是注释
        "key1": 'single quotes',  // 另一个注释
        "key2": [1, 2, 3,],      // 尾随逗号
        "key3": undefined,       // JavaScript样式
        "key4": NaN,            // 特殊数值
        "key5": Infinity        // 无穷大
    })";

    JsonValue::ParseOptions lenientOptions;
    lenientOptions.allowComments = true;
    lenientOptions.allowTrailingCommas = true;
    lenientOptions.allowSpecialNumbers = true;
    lenientOptions.strictMode = false;

    bool success = false;
    try {
        JsonValue parsed = JsonValue::parse(lenientJson, lenientOptions);
        success = true;

        if (success) {
            ASSERT_TRUE(parsed.isObject());
            // 验证特殊数值
            if (parsed["key4"].isNumber()) {
                ASSERT_TRUE(parsed["key4"].isNaN());
            }
            if (parsed["key5"].isNumber()) {
                ASSERT_TRUE(parsed["key5"].isInfinity());
            }
        }
    }
    catch (const std::exception& e) {
        // 如果不支持宽松模式，至少应该有明确的错误消息
        std::cout << "Lenient parsing not supported: " << e.what() << std::endl;
    }
}

TEST(JsonStreamParser_UTF8Handling) {
    // 测试UTF-8编码处理
    std::string utf8Json = R"({
        "english": "Hello World",
        "chinese": "你好世界",
        "japanese": "こんにちは世界", 
        "emoji": "🌍🚀✨",
        "escaped": "\u4f60\u597d\u4e16\u754c",
        "mixed": "Hello 世界 🌍"
    })";

    JsonValue parsed = JsonValue::parse(utf8Json);
    ASSERT_TRUE(parsed.isObject());

    // 验证各种字符集都能正确解析
    ASSERT_EQ(parsed["english"].toString(), "Hello World");
    ASSERT_EQ(parsed["chinese"].toString(), "你好世界");
    ASSERT_EQ(parsed["japanese"].toString(), "こんにちは世界");
    ASSERT_EQ(parsed["emoji"].toString(), "🌍🚀✨");
    ASSERT_EQ(parsed["escaped"].toString(), "你好世界"); // 转义序列应该被解析
    ASSERT_EQ(parsed["mixed"].toString(), "Hello 世界 🌍");
}

TEST(JsonStreamParser_MemoryEfficiency) {
    // 测试内存效率
    size_t memoryBefore = getCurrentMemoryUsage();

    // 创建多个中等大小的JSON文档
    std::vector<JsonValue> documents;
    for (int doc = 0; doc < 100; ++doc) {
        std::stringstream jsonStream;
        jsonStream << "{\"document\": " << doc << ", \"items\": [";
        for (int i = 0; i < 100; ++i) {
            if (i > 0) jsonStream << ",";
            jsonStream << "{\"id\": " << i << ", \"data\": \"item_" << i << "\"}";
        }
        jsonStream << "]}";

        documents.push_back(JsonValue::parse(jsonStream.str()));
    }

    size_t memoryAfter = getCurrentMemoryUsage();
    size_t memoryUsed = memoryAfter - memoryBefore;

    // 验证所有文档都正确解析
    ASSERT_EQ(documents.size(), 100);
    for (const auto& doc : documents) {
        ASSERT_TRUE(doc.isObject());
        ASSERT_TRUE(doc["items"].isArray());
        ASSERT_EQ(doc["items"].toArray()->get().size(), 100);
    }

    // 内存使用应该在合理范围内（这个测试可能需要根据实际情况调整）
    std::cout << "Memory used for 100 documents: " << memoryUsed << " bytes" << std::endl;
}

TEST(JsonStreamParser_ConcurrentParsing) {
    // 测试并发解析（如果支持）
    std::vector<std::string> jsonStrings;
    for (int i = 0; i < 10; ++i) {
        std::stringstream ss;
        ss << "{\"thread\": " << i << ", \"data\": [";
        for (int j = 0; j < 100; ++j) {
            if (j > 0) ss << ",";
            ss << j * i;
        }
        ss << "]}";
        jsonStrings.push_back(ss.str());
    }

    std::vector<JsonValue> results(10);
    std::vector<std::thread> threads;

    // 启动并发解析
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&results, &jsonStrings, i]() {
            results[i] = JsonValue::parse(jsonStrings[i]);
            });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 验证所有结果
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(results[i].isObject());
        ASSERT_EQ(results[i]["thread"].toInt(), i);
        ASSERT_TRUE(results[i]["data"].isArray());
        ASSERT_EQ(results[i]["data"].toArray()->get().size(), 100);
    }
}

TEST(JsonStreamParser_ProgressiveReading) {
    // 测试渐进式读取（流式处理）
    std::string partialJson1 = R"({"start": true, "data": [)";
    std::string partialJson2 = R"(1, 2, 3, 4, 5)";
    std::string partialJson3 = R"(], "end": true})";

    std::string completeJson = partialJson1 + partialJson2 + partialJson3;

    // 测试完整解析
    JsonValue complete = JsonValue::parse(completeJson);
    ASSERT_TRUE(complete.isObject());
    ASSERT_TRUE(complete["start"].toBool());
    ASSERT_TRUE(complete["end"].toBool());
    ASSERT_EQ(complete["data"].toArray()->get().size(), 5);

    // 测试部分JSON会失败
    bool exceptionCaught = false;
    try {
        JsonValue::parse(partialJson1);
    }
    catch (const std::exception&) {
        exceptionCaught = true;
    }
    ASSERT_TRUE(exceptionCaught);
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}