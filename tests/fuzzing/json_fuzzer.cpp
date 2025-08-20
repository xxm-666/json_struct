#include "jsonstruct.h"
#include <string>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

using namespace JsonStruct;

#ifdef FUZZER_NO_LIBFUZZER
// Windows 替代实现：使用随机生成器进行模糊测试
#include <fstream>
#include <chrono>

// 测试函数 - 与 libFuzzer 接口兼容
int FuzzOneInput(const uint8_t *Data, size_t Size) {
    if (Size == 0) {
        return 0;
    }

    std::string json_string(reinterpret_cast<const char*>(Data), Size);
    
    // 1. 测试基础 JSON 解析 (通过 JsonValue 构造函数)
    try {
        JsonValue value(json_string);
        // 基础解析测试
    } catch (...) {
        // 捕获所有预期的解析异常，防止 Fuzzer 因合法错误而停止
    }

    // 2. 测试结构体解析
    struct MyObject {
        JSON_AUTO(name, value)
        std::string name;
        int value = 0;
    };

    try {
        MyObject obj;
        JsonValue json_val(json_string);
        fromJson(obj, json_val);
    } catch (...) {
        // 同样捕获所有异常
    }

    // 3. 测试不同类型的结构体
    struct ComplexObject {
        JSON_AUTO(id, text, flag, numbers)
        int id = 0;
        std::string text;
        bool flag = false;
        std::vector<int> numbers;
    };

    try {
        ComplexObject complex;
        JsonValue json_val(json_string);
        fromJson(complex, json_val);
    } catch (...) {
        // 捕获异常
    }

    return 0;
}

// Windows 主函数实现
int main(int argc, char* argv[]) {
    std::cout << "JSON Fuzzer (Windows Alternative Mode)\n";
    std::cout << "Testing json_struct parsing with random inputs...\n\n";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> byte_dist(0, 255);
    std::uniform_int_distribution<> len_dist(1, 2048);
    
    // 从种子语料库加载测试用例
    std::vector<std::string> seed_inputs;
    const char* seed_files[] = {
        R"({"name": "test", "value": 42})",
        R"({"nested": {"array": [1, 2, 3], "string": "hello world"}})",
        R"([{"id": 1, "data": null}, {"id": 2, "data": true}])",
        R"({"incomplete":)",
        R"({malformed json})",
        "{}",
        "[]",
        "null",
        "true",
        "false",
        "123",
        R"("string")",
        R"({"id": 123, "text": "hello", "flag": true, "numbers": [1,2,3]})"
    };
    
    // 添加种子输入
    for (const auto& seed : seed_files) {
        seed_inputs.push_back(seed);
    }
    
    int iterations = 10000;
    if (argc > 1) {
        iterations = std::atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    
    std::cout << "Running " << iterations << " iterations...\n";
    
    for (int i = 0; i < iterations; ++i) {
        std::vector<uint8_t> data;
        
        if (i < (int)seed_inputs.size() * 100) {
            // 前面的迭代使用种子输入的变种
            const std::string& seed = seed_inputs[i % seed_inputs.size()];
            data.assign(seed.begin(), seed.end());
            
            // 随机修改一些字节
            if (!data.empty() && gen() % 3 == 0) {
                int modifications = gen() % 5 + 1;
                for (int m = 0; m < modifications; ++m) {
                    int pos = gen() % data.size();
                    data[pos] = byte_dist(gen);
                }
            }
        } else {
            // 后续迭代使用完全随机的数据
            int len = len_dist(gen);
            data.resize(len);
            for (int j = 0; j < len; ++j) {
                data[j] = byte_dist(gen);
            }
        }
        
        // 执行测试
        FuzzOneInput(data.data(), data.size());
        
        if (i % 1000 == 0) {
            std::cout << "Completed " << i << " iterations...\n";
        }
    }
    
    std::cout << "\nFuzzing completed! No crashes detected.\n";
    return 0;
}

#else
// 标准 libFuzzer 实现
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size == 0) {
        return 0;
    }

    std::string json_string(reinterpret_cast<const char*>(Data), Size);
    
    // 1. 测试基础 JSON 解析 (通过 JsonValue 构造函数)
    try {
        JsonValue value(json_string);
        // 基础解析测试
    } catch (...) {
        // 捕获所有预期的解析异常，防止 Fuzzer 因合法错误而停止
    }

    // 2. 测试结构体解析
    struct MyObject {
        JSON_AUTO(name, value)
        std::string name;
        int value = 0;
    };

    try {
        MyObject obj;
        JsonValue json_val(json_string);
        fromJson(obj, json_val);
    } catch (...) {
        // 同样捕获所有异常
    }

    // 3. 测试不同类型的结构体
    struct ComplexObject {
        JSON_AUTO(id, text, flag, numbers)
        int id = 0;
        std::string text;
        bool flag = false;
        std::vector<int> numbers;
    };

    try {
        ComplexObject complex;
        JsonValue json_val(json_string);
        fromJson(complex, json_val);
    } catch (...) {
        // 捕获异常
    }

    return 0; // 返回 0 表示执行成功
}
#endif
