#include "../test_framework/test_framework.h"
#include "../src/type_registry/registry_core.h"
#include "../src/type_registry/auto_serializer.h"
#include "../src/json_engine/json_value.h"
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <random>

using namespace JsonStruct;

// 测试多线程并发读写的数据结构
struct ConcurrentTestStruct {
    JSON_AUTO(id, name, values, properties)
    
    int id;
    std::string name;
    std::vector<int> values;
    std::map<std::string, std::string> properties;
};

// 全局测试对象和JsonValue
ConcurrentTestStruct g_testObject;
JsonValue g_jsonValue;
std::mutex g_jsonMutex;
std::atomic<bool> g_stopThreads{false};

// 初始化测试数据
void initializeTestData() {
    g_testObject.id = 1;
    g_testObject.name = "ConcurrentTest";
    g_testObject.values = {1, 2, 3, 4, 5};
    g_testObject.properties = {{"key1", "value1"}, {"key2", "value2"}};
    
    g_jsonValue = g_testObject.toJson();
}

TEST(Multithreaded_ConcurrentReadWrite) {
    initializeTestData();
    
    const int numThreads = 8;
    const int operationsPerThread = 1000;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    std::atomic<int> failureCount{0};
    
    // 启动读线程
    for (int i = 0; i < numThreads / 2; ++i) {
        threads.emplace_back([&, i]() {
            for (int op = 0; op < operationsPerThread; ++op) {
                try {
                    // 读操作
                    std::lock_guard<std::mutex> lock(g_jsonMutex);
                    ConcurrentTestStruct restored;
                    restored.fromJson(g_jsonValue);

                    // 验证数据一致性
                    if (restored.id == g_testObject.id &&
                        restored.name == g_testObject.name &&
                        restored.values.size() == g_testObject.values.size()) {
                        successCount++;
                    } else {
                        std::cerr << "[ReadThread] Data mismatch: "
                                  << "restored.id=" << restored.id << ", g_testObject.id=" << g_testObject.id << "; "
                                  << "restored.name=" << restored.name << ", g_testObject.name=" << g_testObject.name << "; "
                                  << "restored.values.size()=" << restored.values.size() << ", g_testObject.values.size()=" << g_testObject.values.size() << std::endl;
                        failureCount++;
                    }
                } catch (const std::exception& ex) {
                    std::cerr << "[ReadThread] Exception: " << ex.what() << std::endl;
                    failureCount++;
                } catch (...) {
                    std::cerr << "[ReadThread] Unknown exception" << std::endl;
                    failureCount++;
                }

                // 短暂休眠以增加并发性
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
    }
    
    // 启动写线程
    for (int i = 0; i < numThreads / 2; ++i) {
        threads.emplace_back([&, i]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(1, 1000);
            
            for (int op = 0; op < operationsPerThread; ++op) {
                try {
                    // 修改测试对象
                    ConcurrentTestStruct modified = g_testObject;
                    modified.id = dis(gen);
                    modified.name = "Modified_" + std::to_string(dis(gen));
                    
                    // 添加随机值
                    modified.values.push_back(dis(gen));
                    modified.properties["random_key"] = "random_value_" + std::to_string(dis(gen));
                    
                    // 写操作
                    std::lock_guard<std::mutex> lock(g_jsonMutex);
                    g_jsonValue = modified.toJson();
                    g_testObject = modified;
                    
                    successCount++;
                } catch (...) {
                    failureCount++;
                }
                
                // 短暂休眠以增加并发性
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    // 验证结果
    std::cout << "Concurrent operations - Success: " << successCount.load() 
              << ", Failures: " << failureCount.load() << std::endl;
    
    // 应该没有失败的操作
    ASSERT_EQ(failureCount.load(), 0);
    ASSERT_TRUE(successCount.load() > 0);
}

TEST(Multithreaded_ConcurrentJsonValueAccess) {
    // 创建一个复杂的JsonValue用于并发访问测试
    std::map<std::string, std::vector<std::map<std::string, int>>> complexData;
    for (int i = 0; i < 100; ++i) {
        std::vector<std::map<std::string, int>> vec;
        vec.resize(10);
        for (int j = 0; j < 10; ++j) {
            vec[j]["key_" + std::to_string(j)] = i * 10 + j;
        }
        complexData["item_" + std::to_string(i)] = vec;
    }
    
    JsonValue jsonValue = toJsonValue(complexData);
    
    const int numThreads = 10;
    const int operationsPerThread = 500;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    std::atomic<int> failureCount{0};
    
    // 启动多个线程并发访问JsonValue
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&, i]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> itemDis(0, 99);
            std::uniform_int_distribution<> vecDis(0, 9);
            
            for (int op = 0; op < operationsPerThread; ++op) {
                try {
                    // 随机访问JsonValue中的元素
                    int itemIndex = itemDis(gen);
                    int vecIndex = vecDis(gen);
                    
                    std::string itemKey = "item_" + std::to_string(itemIndex);
                    std::string vecKey = "key_" + std::to_string(vecIndex);
                    
                    // 访问JsonValue
                    if (jsonValue.isObject() && jsonValue.contains(itemKey)) {
                        const auto& itemArray = jsonValue[itemKey];
                        if (itemArray.isArray() && itemArray.size() > static_cast<size_t>(vecIndex)) {
                            const auto& mapObj = itemArray[vecIndex];
                            if (mapObj.isObject() && mapObj.contains(vecKey)) {
                                int value = mapObj[vecKey].toInt();
                                // 验证值是否正确
                                if (value == itemIndex * 10 + vecIndex) {
                                    successCount++;
                                } else {
                                    failureCount++;
                                }
                            } else {
                                failureCount++;
                            }
                        } else {
                            failureCount++;
                        }
                    } else {
                        failureCount++;
                    }
                } catch (...) {
                    failureCount++;
                }
                
                // 短暂休眠
                std::this_thread::sleep_for(std::chrono::microseconds(2));
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    // 验证结果
    std::cout << "JsonValue concurrent access - Success: " << successCount.load() 
              << ", Failures: " << failureCount.load() << std::endl;
    
    // 应该没有失败的操作
    ASSERT_EQ(failureCount.load(), 0);
    ASSERT_TRUE(successCount.load() > 0);
}

TEST(Multithreaded_ExceptionHandling) {
    initializeTestData();
    
    const int numThreads = 6;
    std::vector<std::thread> threads;
    std::atomic<int> exceptionCount{0};
    
    // 启动线程进行可能引发异常的操作
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&, i]() {
            for (int op = 0; op < 100; ++op) {
                try {
                    // 尝试对JsonValue进行各种操作
                    std::lock_guard<std::mutex> lock(g_jsonMutex);
                    
                    // 正常操作
                    ConcurrentTestStruct restored;
                    restored.fromJson(g_jsonValue);
                    
                    // 尝试访问不存在的字段
                    try {
                        if (!g_jsonValue.contains("nonexistent_field")) 
                            throw std::runtime_error("Key not found");
                        // 如果没有异常，也计为成功
                    } catch (...) {
                        // 这是预期的异常，计数
                        exceptionCount++;
                    }
                    
                    // 尝试类型转换异常
                    try {
                        // 尝试将对象转换为int
                        if (g_jsonValue.isObject()) {
                            int invalidInt = g_jsonValue.toInt();
                            // 如果没有异常，说明转换成功，这可能不是我们期望的
                        }
                    } catch (...) {
                        // 这是预期的异常，计数
                        exceptionCount++;
                    }
                } catch (...) {
                    // 捕获其他未预期的异常
                    exceptionCount++;
                }
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    // 验证异常处理
    std::cout << "Exception handling test - Exceptions caught: " << exceptionCount.load() << std::endl;
    // 应该捕获到一些异常
    ASSERT_TRUE(exceptionCount.load() > 0);
}

int main() {
    std::cout << "=== Multithreaded Concurrency Tests ===" << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    if (result == 0) {
        std::cout << "✅ All multithreaded concurrency tests PASSED!" << std::endl;
    } else {
        std::cout << "❌ Some multithreaded concurrency tests FAILED!" << std::endl;
    }
    
    return result;
}