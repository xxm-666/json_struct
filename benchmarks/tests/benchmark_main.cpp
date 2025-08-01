// JsonStruct Benchmark Framework
// This file is the entry point for all benchmarks
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <thread>
#include <mutex>
#include "json_engine/json_value.h"
#include "json_engine/json_filter.h"
#include "json_engine/json_path.h"
#include "json_engine/json_pipeline.h"
#include "json_engine/json_query_generator.h"
#include "json_engine/json_patch.h"

using namespace JsonStruct;

struct BenchmarkCase {
    std::string name;
    std::function<void()> run;
};

struct BenchmarkResult {
    std::string name;
    long long totalTime;
};

std::vector<BenchmarkResult> g_results;
std::mutex g_results_mutex;

void save_results_csv(const std::vector<BenchmarkResult>& results, const std::string& filename) {
    std::ofstream out(filename);
    out << "Benchmark,TotalTime(us)\n";
    for (const auto& r : results) {
        out << r.name << "," << r.totalTime << "\n";
    }
}

void run_benchmark(const std::string& name, const std::function<void()>& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    auto total = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "\n=== " << name << " ===" << std::endl;
    std::cout << "Total Time: " << total << " microseconds" << std::endl;
    std::lock_guard<std::mutex> lock(g_results_mutex);
    g_results.push_back({name, total});
}

// Multi-threaded serialization benchmark
void serialization_mt() {
    JsonValue json = JsonValue::parse("{\"name\": \"John\", \"age\": 30, \"city\": \"New York\"}");
    int threads = std::thread::hardware_concurrency();
    int iterations = 10000;
    std::vector<std::thread> workers;
    for (int t = 0; t < threads; ++t) {
        workers.emplace_back([&]() {
            for (int i = 0; i < iterations; ++i) {
                std::string s = json.toString();
            }
        });
    }
    for (auto& th : workers) th.join();
}

int main() {
    std::vector<BenchmarkCase> cases;
    cases.push_back({"Serialization", []() {
        JsonValue json = JsonValue::parse("{\"name\": \"John\", \"age\": 30, \"city\": \"New York\"}");
        for (int i = 0; i < 10000; ++i) {
            std::string s = json.toString();
        }
    }});
    cases.push_back({"Deserialization", []() {
        std::string str = "{\"name\": \"John\", \"age\": 30, \"city\": \"New York\"}";
        for (int i = 0; i < 10000; ++i) {
            JsonValue json = JsonValue::parse(str);
        }
    }});
    cases.push_back({"Pipeline", []() {
        JsonValue json = JsonValue::parse("{\"numbers\": [1,2,3,4,5]}");
        JsonPipeline pipeline;
        pipeline.transform([](const JsonValue& v) { return v; });
        for (int i = 0; i < 10000; ++i) {
            pipeline.execute(json);
        }
    }});
    cases.push_back({"JSONPath", []() {
        JsonValue json = JsonValue::parse("{\"store\": {\"book\": [{\"author\": \"Author1\"}]}}");
        JsonPipeline pipeline;
        auto queryFunc = pipeline.query("$.store.book[*].author");
        for (int i = 0; i < 10000; ++i) {
            JsonValue result = queryFunc(json);
        }
    }});
    cases.push_back({"Memory Usage", []() {
        std::vector<JsonValue> vec;
        for (int i = 0; i < 10000; ++i) {
            vec.push_back(JsonValue::parse("{\"id\": " + std::to_string(i) + "}"));
        }
        std::cout << "Allocated " << vec.size() << " JsonValue objects." << std::endl;
    }});
    cases.push_back({"Serialization_MT", serialization_mt});
    cases.push_back({"Complex Serialization", []() {
        JsonValue json = JsonValue::parse(R"({
            "user": {
                "id": 123,
                "name": "Alice",
                "roles": ["admin", "editor"],
                "profile": {
                    "age": 29,
                    "address": {
                        "city": "Wonderland",
                        "zip": "12345"
                    }
                }
            },
            "logs": [
                {"action": "login", "timestamp": "2025-07-24T10:00:00Z"},
                {"action": "update", "timestamp": "2025-07-24T10:05:00Z"}
            ]
        })");
        for (int i = 0; i < 10000; ++i) {
            std::string s = json.toString();
        }
    }});
    cases.push_back({"Complex Deserialization", []() {
        std::string str = R"({
            "user": {
                "id": 123,
                "name": "Alice",
                "roles": ["admin", "editor"],
                "profile": {
                    "age": 29,
                    "address": {
                        "city": "Wonderland",
                        "zip": "12345"
                    }
                }
            },
            "logs": [
                {"action": "login", "timestamp": "2025-07-24T10:00:00Z"},
                {"action": "update", "timestamp": "2025-07-24T10:05:00Z"}
            ]
        })";
        for (int i = 0; i < 10000; ++i) {
            JsonValue json = JsonValue::parse(str);
        }
    }});
    cases.push_back({"Filter", []() {
        JsonValue json = JsonValue::parse("{\"numbers\": [1,2,3,4,5,6,7,8,9,10]}");
        for (int i = 0; i < 10000; ++i) {
            // Filter even numbers
            auto arr = json["numbers"].toArray();
            std::vector<int> evens;
            for (const auto& v : arr) {
                if (v.toInt() % 2 == 0) evens.push_back(v.toInt());
            }
        }
    }});
    cases.push_back({"Patch", []() {
        JsonValue json = JsonValue::parse("{\"name\": \"John\", \"age\": 30}");
        JsonValue patch = JsonValue::parse("[{\"op\": \"replace\", \"path\": \"/name\", \"value\": \"Jane\"}]");
        for (int i = 0; i < 10000; ++i) {
            JsonStruct::JsonPatch::ApplyPatch(json, patch);
        }
    }});
    cases.push_back({"Query Generator", []() {
        JsonValue root = JsonValue::parse("{\"store\": {\"book\": [{\"author\": \"Author1\"}]}}");
        JsonQueryGenerator gen(root, "$.store.book[*].author", {});
        while (gen.hasMore()) { gen.getNext(); }
    }});
    for (const auto& c : cases) {
        run_benchmark(c.name, c.run);
    }
    save_results_csv(g_results, "benchmark_results.csv");
    return 0;
}
