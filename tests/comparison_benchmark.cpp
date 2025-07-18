#include "../src/jsonstruct.h"
#include "benchmark_framework.h"
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <iomanip>

// 根据构建配置选择使用真实库还是模拟库
#ifdef USE_REAL_JSON_LIBRARIES
    #include <nlohmann/json.hpp>
    #include <rapidjson/document.h>
    #include <rapidjson/writer.h>
    #include <rapidjson/stringbuffer.h>
    #include <json/json.h>
    #define USING_REAL_LIBRARIES true
#else
    #define USING_REAL_LIBRARIES false
#endif

using namespace JsonStruct;
using namespace BenchmarkFramework;

// === 测试数据加载器 ===
class TestDataLoader {
public:
    static std::vector<std::string> getTestFiles() {
        std::vector<std::string> files;
        std::vector<std::string> candidates = {
            "complex_data_1mb.json",
            "complex_data_10mb.json", 
            "complex_data_50mb.json",
            "complex_data_100mb.json",
            "complex_data_json5_20mb.json",
            "complex_data_json5_80mb.json"
        };
        
        for (const auto& file : candidates) {
            if (std::filesystem::exists(file)) {
                files.push_back(file);
            }
        }
        
        return files;
    }
    
    static std::string loadFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    
    static size_t getFileSize(const std::string& filename) {
        return std::filesystem::file_size(filename);
    }
};

// === 基础测试结构体 ===
struct ComparisonStruct {
    int id = 12345;
    std::string name = "performance_test_object";
    double value = 98.765;
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::map<std::string, std::string> properties = {
        {"type", "benchmark"},
        {"category", "performance"}, 
        {"version", "1.0.0"},
        {"status", "active"}
    };
    
    JSON_AUTO(id, name, value, numbers, properties)
};

#ifdef USE_REAL_JSON_LIBRARIES

// === 真实库实现 ===

namespace RealLibTests {

// nlohmann/json 测试
class NlohmannTest {
public:
    static std::string serialize(const ComparisonStruct& obj) {
        nlohmann::json j;
        j["id"] = obj.id;
        j["name"] = obj.name;
        j["value"] = obj.value;
        j["numbers"] = obj.numbers;
        j["properties"] = obj.properties;
        return j.dump();
    }
    
    static ComparisonStruct deserialize(const std::string& jsonStr) {
        auto j = nlohmann::json::parse(jsonStr);
        ComparisonStruct obj;
        obj.id = j["id"];
        obj.name = j["name"];
        obj.value = j["value"];
        obj.numbers = j["numbers"];
        obj.properties = j["properties"];
        return obj;
    }
    
    static bool parseFile(const std::string& content) {
        try {
            auto j = nlohmann::json::parse(content);
            return !j.empty();
        } catch (...) {
            return false;
        }
    }
};

// RapidJSON 测试
class RapidJsonTest {
public:
    static std::string serialize(const ComparisonStruct& obj) {
        rapidjson::Document d;
        d.SetObject();
        auto& allocator = d.GetAllocator();
        
        d.AddMember("id", obj.id, allocator);
        d.AddMember("name", rapidjson::Value(obj.name.c_str(), allocator), allocator);
        d.AddMember("value", obj.value, allocator);
        
        rapidjson::Value numbers(rapidjson::kArrayType);
        for (int num : obj.numbers) {
            numbers.PushBack(num, allocator);
        }
        d.AddMember("numbers", numbers, allocator);
        
        rapidjson::Value props(rapidjson::kObjectType);
        for (const auto& [key, val] : obj.properties) {
            rapidjson::Value k(key.c_str(), allocator);
            rapidjson::Value v(val.c_str(), allocator);
            props.AddMember(k, v, allocator);
        }
        d.AddMember("properties", props, allocator);
        
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        d.Accept(writer);
        return buffer.GetString();
    }
    
    static ComparisonStruct deserialize(const std::string& jsonStr) {
        rapidjson::Document d;
        d.Parse(jsonStr.c_str());
        
        ComparisonStruct obj;
        if (d.HasMember("id")) obj.id = d["id"].GetInt();
        if (d.HasMember("name")) obj.name = d["name"].GetString();
        if (d.HasMember("value")) obj.value = d["value"].GetDouble();
        
        if (d.HasMember("numbers") && d["numbers"].IsArray()) {
            obj.numbers.clear();
            for (auto& v : d["numbers"].GetArray()) {
                obj.numbers.push_back(v.GetInt());
            }
        }
        
        if (d.HasMember("properties") && d["properties"].IsObject()) {
            obj.properties.clear();
            for (auto& m : d["properties"].GetObject()) {
                obj.properties[m.name.GetString()] = m.value.GetString();
            }
        }
        
        return obj;
    }
    
    static bool parseFile(const std::string& content) {
        rapidjson::Document d;
        d.Parse(content.c_str());
        return !d.HasParseError();
    }
};

// jsoncpp 测试
class JsonCppTest {
public:
    static std::string serialize(const ComparisonStruct& obj) {
        Json::Value root;
        root["id"] = obj.id;
        root["name"] = obj.name;
        root["value"] = obj.value;
        
        Json::Value numbers(Json::arrayValue);
        for (int num : obj.numbers) {
            numbers.append(num);
        }
        root["numbers"] = numbers;
        
        Json::Value props(Json::objectValue);
        for (const auto& [key, val] : obj.properties) {
            props[key] = val;
        }
        root["properties"] = props;
        
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        return Json::writeString(builder, root);
    }
    
    static ComparisonStruct deserialize(const std::string& jsonStr) {
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string errors;
        std::istringstream stream(jsonStr);
        
        if (!Json::parseFromStream(builder, stream, &root, &errors)) {
            throw std::runtime_error("JSON parse error: " + errors);
        }
        
        ComparisonStruct obj;
        if (root.isMember("id")) obj.id = root["id"].asInt();
        if (root.isMember("name")) obj.name = root["name"].asString();
        if (root.isMember("value")) obj.value = root["value"].asDouble();
        
        if (root.isMember("numbers") && root["numbers"].isArray()) {
            obj.numbers.clear();
            for (const auto& v : root["numbers"]) {
                obj.numbers.push_back(v.asInt());
            }
        }
        
        if (root.isMember("properties") && root["properties"].isObject()) {
            obj.properties.clear();
            for (const auto& key : root["properties"].getMemberNames()) {
                obj.properties[key] = root["properties"][key].asString();
            }
        }
        
        return obj;
    }
    
    static bool parseFile(const std::string& content) {
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string errors;
        std::istringstream stream(content);
        return Json::parseFromStream(builder, root, &errors);
    }
};

} // namespace RealLibTests

#else

// === 模拟库实现（fallback） ===

namespace MockLibTests {

class NlohmannTest {
public:
    static std::string serialize(const ComparisonStruct& obj) {
        std::ostringstream oss;
        oss << "{\"id\":" << obj.id << ",\"name\":\"" << obj.name 
            << "\",\"value\":" << obj.value << ",\"numbers\":[";
        for (size_t i = 0; i < obj.numbers.size(); ++i) {
            if (i > 0) oss << ",";
            oss << obj.numbers[i];
        }
        oss << "],\"properties\":{";
        bool first = true;
        for (const auto& [key, val] : obj.properties) {
            if (!first) oss << ",";
            oss << "\"" << key << "\":\"" << val << "\"";
            first = false;
        }
        oss << "}}";
        return oss.str();
    }
    
    static ComparisonStruct deserialize(const std::string& jsonStr) {
        // 模拟反序列化
        return ComparisonStruct{};
    }
    
    static bool parseFile(const std::string& content) {
        return !content.empty() && content[0] == '{';
    }
};

class RapidJsonTest {
public:
    static std::string serialize(const ComparisonStruct& obj) {
        return NlohmannTest::serialize(obj);
    }
    
    static ComparisonStruct deserialize(const std::string& jsonStr) {
        return ComparisonStruct{};
    }
    
    static bool parseFile(const std::string& content) {
        return !content.empty() && content[0] == '{';
    }
};

class JsonCppTest {
public:
    static std::string serialize(const ComparisonStruct& obj) {
        return NlohmannTest::serialize(obj);
    }
    
    static ComparisonStruct deserialize(const std::string& jsonStr) {
        return ComparisonStruct{};
    }
    
    static bool parseFile(const std::string& content) {
        return !content.empty() && content[0] == '{';
    }
};

} // namespace MockLibTests

// 使用模拟库的别名
namespace RealLibTests = MockLibTests;

#endif

// === 基准测试实现 ===

// 小结构体序列化测试
BENCHMARK(JsonStruct_SmallStruct_Serialization) {
    ComparisonStruct testData;
    
    return Benchmark("JsonStruct小结构序列化")
        .iterations(10000)
        .warmup(100)
        .run([&]() {
            auto json = testData.toJson();
            std::string str = json.dump();
            volatile size_t len = str.length();
            (void)len;
        });
}

BENCHMARK(Nlohmann_SmallStruct_Serialization) {
    ComparisonStruct testData;
    
    return Benchmark(std::string(USING_REAL_LIBRARIES ? "Nlohmann" : "模拟Nlohmann") + "小结构序列化")
        .iterations(10000)
        .warmup(100)
        .run([&]() {
            std::string json = RealLibTests::NlohmannTest::serialize(testData);
            volatile size_t len = json.length();
            (void)len;
        });
}

BENCHMARK(RapidJSON_SmallStruct_Serialization) {
    ComparisonStruct testData;
    
    return Benchmark(std::string(USING_REAL_LIBRARIES ? "RapidJSON" : "模拟RapidJSON") + "小结构序列化")
        .iterations(10000)
        .warmup(100)
        .run([&]() {
            std::string json = RealLibTests::RapidJsonTest::serialize(testData);
            volatile size_t len = json.length();
            (void)len;
        });
}

BENCHMARK(JsonCpp_SmallStruct_Serialization) {
    ComparisonStruct testData;
    
    return Benchmark(std::string(USING_REAL_LIBRARIES ? "JsonCpp" : "模拟JsonCpp") + "小结构序列化")
        .iterations(10000)
        .warmup(100)
        .run([&]() {
            std::string json = RealLibTests::JsonCppTest::serialize(testData);
            volatile size_t len = json.length();
            (void)len;
        });
}

// 小结构体反序列化测试
BENCHMARK(JsonStruct_SmallStruct_Deserialization) {
    ComparisonStruct original;
    auto jsonVal = original.toJson();
    std::string jsonStr = jsonVal.dump();
    
    return Benchmark("JsonStruct小结构反序列化")
        .iterations(10000)
        .warmup(100)
        .run([&]() {
            ComparisonStruct restored;
            restored.fromJson(JsonValue::parse(jsonStr));
            volatile int id = restored.id;
            (void)id;
        });
}

BENCHMARK(Nlohmann_SmallStruct_Deserialization) {
    ComparisonStruct original;
    auto jsonVal = original.toJson();
    std::string jsonStr = jsonVal.dump();
    
    return Benchmark(std::string(USING_REAL_LIBRARIES ? "Nlohmann" : "模拟Nlohmann") + "小结构反序列化")
        .iterations(10000)
        .warmup(100)
        .run([&]() {
            ComparisonStruct restored = RealLibTests::NlohmannTest::deserialize(jsonStr);
            volatile int id = restored.id;
            (void)id;
        });
}

BENCHMARK(RapidJSON_SmallStruct_Deserialization) {
    ComparisonStruct original;
    auto jsonVal = original.toJson();
    std::string jsonStr = jsonVal.dump();
    
    return Benchmark(std::string(USING_REAL_LIBRARIES ? "RapidJSON" : "模拟RapidJSON") + "小结构反序列化")
        .iterations(10000)
        .warmup(100)
        .run([&]() {
            ComparisonStruct restored = RealLibTests::RapidJsonTest::deserialize(jsonStr);
            volatile int id = restored.id;
            (void)id;
        });
}

BENCHMARK(JsonCpp_SmallStruct_Deserialization) {
    ComparisonStruct original;
    auto jsonVal = original.toJson();
    std::string jsonStr = jsonVal.dump();
    
    return Benchmark(std::string(USING_REAL_LIBRARIES ? "JsonCpp" : "模拟JsonCpp") + "小结构反序列化")
        .iterations(10000)
        .warmup(100)
        .run([&]() {
            ComparisonStruct restored = RealLibTests::JsonCppTest::deserialize(jsonStr);
            volatile int id = restored.id;
            (void)id;
        });
}

// 大文件解析测试
void runLargeFileTests() {
    auto testFiles = TestDataLoader::getTestFiles();
    
    if (testFiles.empty()) {
        std::cout << "⚠️  未找到测试JSON文件，跳过大文件测试" << std::endl;
        std::cout << "⚠️  No test JSON files found, skipping large file tests" << std::endl;
        return;
    }
    
    std::cout << "\n=== 大文件解析性能测试 Large File Parsing Performance ===" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    for (const auto& filename : testFiles) {
        try {
            std::string content = TestDataLoader::loadFile(filename);
            size_t fileSize = TestDataLoader::getFileSize(filename);
            
            std::cout << "\n测试文件 Test File: " << filename 
                      << " (Size: " << fileSize / 1024 << " KB)" << std::endl;
            std::cout << std::string(60, '-') << std::endl;
            
            // JsonStruct 解析测试
            auto jsResult = Benchmark("JsonStruct大文件解析")
                .iterations(10)
                .warmup(2)
                .run([&]() {
                    try {
                        auto json = JsonValue::parse(content);
                        volatile bool valid = !json.isNull();
                        (void)valid;
                    } catch (...) {
                        // 解析失败
                    }
                });
            BenchmarkManager::getInstance().printResult(jsResult);
            
            // nlohmann/json 解析测试
            auto nlResult = Benchmark(std::string(USING_REAL_LIBRARIES ? "Nlohmann" : "模拟Nlohmann") + "大文件解析")
                .iterations(10) 
                .warmup(2)
                .run([&]() {
                    bool result = RealLibTests::NlohmannTest::parseFile(content);
                    volatile bool valid = result;
                    (void)valid;
                });
            BenchmarkManager::getInstance().printResult(nlResult);
            
            // RapidJSON 解析测试
            auto rpResult = Benchmark(std::string(USING_REAL_LIBRARIES ? "RapidJSON" : "模拟RapidJSON") + "大文件解析")
                .iterations(10)
                .warmup(2)
                .run([&]() {
                    bool result = RealLibTests::RapidJsonTest::parseFile(content);
                    volatile bool valid = result;
                    (void)valid;
                });
            BenchmarkManager::getInstance().printResult(rpResult);
            
            // jsoncpp 解析测试
            auto jcResult = Benchmark(std::string(USING_REAL_LIBRARIES ? "JsonCpp" : "模拟JsonCpp") + "大文件解析")
                .iterations(10)
                .warmup(2)
                .run([&]() {
                    bool result = RealLibTests::JsonCppTest::parseFile(content);
                    volatile bool valid = result;
                    (void)valid;
                });
            BenchmarkManager::getInstance().printResult(jcResult);
            
        } catch (const std::exception& e) {
            std::cout << "❌ 文件测试失败 File test failed: " << e.what() << std::endl;
        }
    }
}

// === 代码复杂度对比分析 ===

void printCodeComplexityComparison() {
    std::cout << "\n=== 代码复杂度对比 Code Complexity Comparison ===" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    std::cout << "为相同功能编写序列化代码所需的代码行数:" << std::endl;
    std::cout << "Lines of code required for equivalent serialization functionality:" << std::endl;
    std::cout << std::endl;
    
    std::cout << "JsonStruct:" << std::endl;
    std::cout << "  struct MyStruct {" << std::endl;
    std::cout << "      int id; std::string name; double value;" << std::endl;
    std::cout << "      JSON_AUTO(id, name, value)  // 1行!" << std::endl;
    std::cout << "  };" << std::endl;
    std::cout << "  总计: ~4行代码 Total: ~4 lines" << std::endl;
    std::cout << std::endl;
    
    std::cout << "传统方式 (nlohmann/json风格):" << std::endl;
    std::cout << "  struct MyStruct { int id; std::string name; double value; };" << std::endl;
    std::cout << "  void to_json(json& j, const MyStruct& s) {" << std::endl;
    std::cout << "      j = json{{\"id\", s.id}, {\"name\", s.name}, {\"value\", s.value}};" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << "  void from_json(const json& j, MyStruct& s) {" << std::endl;
    std::cout << "      j.at(\"id\").get_to(s.id);" << std::endl;
    std::cout << "      j.at(\"name\").get_to(s.name);" << std::endl;
    std::cout << "      j.at(\"value\").get_to(s.value);" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << "  总计: ~9行代码 Total: ~9 lines" << std::endl;
    std::cout << std::endl;
    
    std::cout << "手动序列化 (RapidJSON/jsoncpp风格):" << std::endl;
    std::cout << "  需要手动构建JSON对象和解析逻辑" << std::endl;
    std::cout << "  Manual JSON object construction and parsing logic required" << std::endl;
    std::cout << "  总计: ~15-25行代码 Total: ~15-25 lines" << std::endl;
    std::cout << std::endl;
    
    std::cout << "代码简化率 Code Reduction:" << std::endl;
    std::cout << "  相比nlohmann/json: 减少 ~56% (4 vs 9行)" << std::endl;
    std::cout << "  相比手动序列化: 减少 ~75% (4 vs 16行)" << std::endl;
    std::cout << "  Compared to nlohmann/json: ~56% reduction (4 vs 9 lines)" << std::endl;
    std::cout << "  Compared to manual serialization: ~75% reduction (4 vs 16 lines)" << std::endl;
}

// === 功能特性对比 ===

void printFeatureComparison() {
    std::cout << "\n=== 功能特性对比 Feature Comparison ===" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    std::cout << std::left << std::setw(35) << "特性 Feature" 
              << std::setw(12) << "JsonStruct" 
              << std::setw(12) << "nlohmann"
              << std::setw(12) << "RapidJSON"
              << std::setw(12) << "jsoncpp" << std::endl;
    std::cout << std::string(83, '-') << std::endl;
    
    std::vector<std::tuple<std::string, bool, bool, bool, bool>> features = {
        {"自动序列化 Auto Serialization", true, false, false, false},
        {"STL容器支持 STL Container Support", true, true, false, false},
        {"Qt类型支持 Qt Type Support", true, false, false, false},
        {"JSONPath查询 JSONPath Query", true, false, false, false},
        {"版本管理 Version Management", true, false, false, false},
        {"编译时类型检查 Compile-time Type Check", true, false, false, false},
        {"零拷贝优化 Zero-copy Optimization", true, true, true, false},
        {"成熟度 Maturity", false, true, true, true},
        {"社区支持 Community Support", false, true, true, true},
        {"文档完整性 Documentation", false, true, true, true}
    };
    
    for (const auto& [feature, js, nl, rj, jc] : features) {
        std::cout << std::left << std::setw(35) << feature
                  << std::setw(12) << (js ? "✅" : "❌")
                  << std::setw(12) << (nl ? "✅" : "❌")
                  << std::setw(12) << (rj ? "✅" : "❌")
                  << std::setw(12) << (jc ? "✅" : "❌") << std::endl;
    }
}

int main() {
    std::cout << "JsonStruct vs 主流JSON库性能对比测试" << std::endl;
    std::cout << "JsonStruct vs Popular JSON Libraries Performance Comparison" << std::endl;
    
    if (USING_REAL_LIBRARIES) {
        std::cout << "✅ 使用真实的第三方JSON库进行对比测试" << std::endl;
        std::cout << "✅ Using real third-party JSON libraries for comparison" << std::endl;
    } else {
        std::cout << "⚠️  使用模拟库进行对比，实际结果可能有差异" << std::endl;
        std::cout << "⚠️  Using mock libraries for comparison, actual results may vary" << std::endl;
    }
    
    // 运行基准测试
    RUN_ALL_BENCHMARKS();
    
    // 运行大文件测试
    runLargeFileTests();
    
    // 打印对比分析
    printCodeComplexityComparison();
    printFeatureComparison();
    
    std::cout << "\n=== 总结 Summary ===" << std::endl;
    std::cout << "JsonStruct的主要优势 Main advantages of JsonStruct:" << std::endl;
    std::cout << "1. 极简的API设计 - JSON_AUTO()一行完成注册" << std::endl;
    std::cout << "2. 原生STL和Qt类型支持，无需手动适配" << std::endl;
    std::cout << "3. 独特的JSONPath查询功能" << std::endl;
    std::cout << "4. 编译时类型安全检查" << std::endl;
    std::cout << "5. 内置版本管理系统" << std::endl;
    std::cout << std::endl;
    std::cout << "适用场景 Suitable scenarios:" << std::endl;
    std::cout << "- 新项目开发，追求现代C++特性" << std::endl;
    std::cout << "- 需要JSONPath查询功能的应用" << std::endl;
    std::cout << "- STL/Qt重度使用的项目" << std::endl;
    std::cout << "- 希望减少序列化样板代码的团队" << std::endl;
    
    return 0;
}
