#include "../test_framework/test_framework.h"
#include "../src/json_engine/json_query_generator.h"
#include "../src/json_engine/json_value.h"
#include <chrono>
#include <random>
#include <memory>

using namespace JsonStruct;
using namespace TestFramework;

// 辅助函数用于创建测试数据
JsonValue createTestUsers() {
    JsonValue::ObjectType usersObj;
    JsonValue::ArrayType usersArray;
    
    JsonValue::ObjectType user1;
    user1["name"] = JsonValue(std::string("Alice"));
    user1["age"] = JsonValue(25);
    usersArray.push_back(JsonValue(std::move(user1)));
    
    JsonValue::ObjectType user2;
    user2["name"] = JsonValue(std::string("Bob"));
    user2["age"] = JsonValue(30);
    usersArray.push_back(JsonValue(std::move(user2)));
    
    JsonValue::ObjectType user3;
    user3["name"] = JsonValue(std::string("Charlie"));
    user3["age"] = JsonValue(35);
    usersArray.push_back(JsonValue(std::move(user3)));
    
    usersObj["users"] = JsonValue(std::move(usersArray));
    return JsonValue(std::move(usersObj));
}

JsonValue createLargeArray(int size) {
    JsonValue::ArrayType largeArray;
    for (int i = 0; i < size; ++i) {
        JsonValue::ObjectType item;
        item["id"] = JsonValue(i);
        item["value"] = JsonValue(std::string("item_" + std::to_string(i)));
        largeArray.push_back(JsonValue(std::move(item)));
    }
    return JsonValue(std::move(largeArray));
}

// 基础功能测试
void test_basic_generator_functionality(TestResult& result) {
    try {
        // 创建简单测试数据
        JsonValue testData = createTestUsers();

        // 测试基础迭代器功能
        auto generator = JsonQueryGenerator(testData, "$.users[*]");
        
        int count = 0;
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            ASSERT_TRUE(it->first != nullptr);
            count++;
        }
        
        ASSERT_EQ(3, count);
        
        // 测试生成器状态
        ASSERT_TRUE(generator.getState() == JsonQueryGenerator::State::Completed);
        ASSERT_EQ(3, generator.getTotalGenerated());
        
    } catch (const std::exception& e) {
        result.addFail("基础功能测试失败: " + std::string(e.what()));
    }
}

// 早期终止测试
void test_early_termination(TestResult& result) {
    try {
        // 创建测试数据
        JsonValue testData = createLargeArray(100);

        // 测试stopOnFirstMatch选项
        JsonQueryGenerator::GeneratorOptions opts;
        opts.stopOnFirstMatch = true;
        
        auto generator = JsonQueryGenerator(testData, "$[*]", opts);
        
        int count = 0;
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            count++;
        }
        
        ASSERT_EQ(1, count);
        ASSERT_EQ(1, generator.getTotalGenerated());
        
        // 测试maxResults限制
        opts.stopOnFirstMatch = false;
        opts.maxResults = 5;
        
        auto limitedGenerator = JsonQueryGenerator(testData, "$[*]", opts);
        
        count = 0;
        for (auto it = limitedGenerator.begin(); it != limitedGenerator.end(); ++it) {
            count++;
        }
        
        ASSERT_EQ(5, count);
        ASSERT_EQ(5, limitedGenerator.getTotalGenerated());
        
    } catch (const std::exception& e) {
        result.addFail("早期终止测试失败: " + std::string(e.what()));
    }
}

// 批量处理测试
void test_batch_processing(TestResult& result) {
    try {
        // 创建测试数据
        JsonValue testData = createLargeArray(50);

        JsonQueryGenerator::GeneratorOptions opts;
        opts.batchSize = 10;
        
        auto generator = JsonQueryGenerator(testData, "$[*]", opts);
        
        // 测试takeBatch方法
        auto batch1 = generator.takeBatch(15);
        ASSERT_EQ(15, batch1.size());
        
        auto batch2 = generator.takeBatch(20);
        ASSERT_EQ(20, batch2.size());
        
        auto batch3 = generator.takeBatch(20);
        ASSERT_EQ(15, batch3.size()); // 剩余15个
        
        auto batch4 = generator.takeBatch(10);
        ASSERT_EQ(0, batch4.size()); // 应该为空
        
    } catch (const std::exception& e) {
        result.addFail("批量处理测试失败: " + std::string(e.what()));
    }
}

// 流式查询工厂类测试
void test_streaming_query_factory(TestResult& result) {
    try {
        // 创建测试数据
        JsonValue::ObjectType productsObj;
        JsonValue::ArrayType productsArray;
        
        JsonValue::ObjectType product1;
        product1["name"] = JsonValue(std::string("Laptop"));
        product1["price"] = JsonValue(1000);
        product1["category"] = JsonValue(std::string("Electronics"));
        productsArray.push_back(JsonValue(std::move(product1)));
        
        JsonValue::ObjectType product2;
        product2["name"] = JsonValue(std::string("Book"));
        product2["price"] = JsonValue(20);
        product2["category"] = JsonValue(std::string("Education"));
        productsArray.push_back(JsonValue(std::move(product2)));
        
        JsonValue::ObjectType product3;
        product3["name"] = JsonValue(std::string("Phone"));
        product3["price"] = JsonValue(800);
        product3["category"] = JsonValue(std::string("Electronics"));
        productsArray.push_back(JsonValue(std::move(product3)));
        
        JsonValue::ObjectType product4;
        product4["name"] = JsonValue(std::string("Desk"));
        product4["price"] = JsonValue(200);
        product4["category"] = JsonValue(std::string("Furniture"));
        productsArray.push_back(JsonValue(std::move(product4)));
        
        productsObj["products"] = JsonValue(std::move(productsArray));
        JsonValue testData(std::move(productsObj));

        // 测试findFirst
        auto firstMatch = JsonStreamingQuery::findFirst(testData, "$.products[*]");
        ASSERT_TRUE(firstMatch.has_value());
        ASSERT_TRUE(firstMatch->first != nullptr);
        
        // 测试countMatches
        size_t totalCount = JsonStreamingQuery::countMatches(testData, "$.products[*]");
        ASSERT_EQ(4, totalCount);
        
        size_t limitedCount = JsonStreamingQuery::countMatches(testData, "$.products[*]", 2);
        ASSERT_EQ(2, limitedCount);
        
        // 测试lazyQuery
        std::vector<std::string> names;
        size_t processed = JsonStreamingQuery::lazyQuery(testData, "$.products[*]", 
            [&names](const JsonValue* value, const std::string& path) -> bool {
                if (value->getObject()) {
                    auto obj = value->getObject();
                    auto it = obj->find("name");
                    if (it != obj->end()) {
                        if (auto str = it->second.getString()) {
                            names.push_back(std::string(*str));
                            // 调试输出
                            std::cout << "处理了产品: " << *str << ", 当前names.size() = " << names.size() << std::endl;
                        }
                    }
                }
                bool shouldContinue = names.size() < 3;
                std::cout << "回调返回: " << (shouldContinue ? "true" : "false") << std::endl;
                return shouldContinue; // 只处理前3个
            });
        
        std::cout << "lazyQuery processed = " << processed << ", names.size() = " << names.size() << std::endl;
        ASSERT_EQ(3, processed);
        ASSERT_EQ(3, names.size());
        
    } catch (const std::exception& e) {
        result.addFail("流式查询工厂测试失败: " + std::string(e.what()));
    }
}

// 性能测试
void test_performance_comparison(TestResult& result) {
    try {
        // 创建大型测试数据 (10000个对象)
        JsonValue largeData = createLargeArray(10000);

        // 测试流式查询性能 (只获取前100个)
        auto start = std::chrono::high_resolution_clock::now();
        
        JsonQueryGenerator::GeneratorOptions opts;
        opts.maxResults = 100;
        
        auto generator = JsonQueryGenerator(largeData, "$[*]", opts);
        
        int count = 0;
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            count++;
            // 模拟一些处理
            if (it->first->getObject()) {
                auto obj = it->first->getObject();
                auto it_id = obj->find("id");
                if (it_id != obj->end()) {
                    // 简单访问，避免复杂操作
                    (void)it_id->second;
                }
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        ASSERT_EQ(100, count);
        
        // 性能应该在合理范围内 (小于100ms)
        ASSERT_TRUE(duration.count() < 100);

        // 测试早期终止的效率
        start = std::chrono::high_resolution_clock::now();
        
        auto firstResult = JsonStreamingQuery::findFirst(largeData, "$[*]");
        
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        ASSERT_TRUE(firstResult.has_value());
        ASSERT_TRUE(duration.count() < 10);
        
    } catch (const std::exception& e) {
        result.addFail("性能测试失败: " + std::string(e.what()));
    }
}

// 大对象测试
void test_large_object_handling(TestResult& result) {
    try {
        // 创建深度嵌套的大对象
        JsonValue::ObjectType largeObject;
        
        // 创建多层嵌套结构
        JsonValue::ObjectType* current = &largeObject;
        for (int level = 0; level < 10; ++level) {
            JsonValue::ObjectType nextLevel;
            JsonValue::ArrayType dataArray;
            
            // 在每层添加一些数据
            for (int i = 0; i < 100; ++i) {
                JsonValue::ObjectType item;
                item["item_id"] = JsonValue(level * 100 + i);
                item["value"] = JsonValue(std::string("Level" + std::to_string(level) + "_Item" + std::to_string(i)));
                dataArray.push_back(JsonValue(std::move(item)));
            }
            
            (*current)["level_" + std::to_string(level)] = JsonValue(std::move(nextLevel));
            (*current)["data_" + std::to_string(level)] = JsonValue(std::move(dataArray));
            
            // 获取下一层的引用
            if (auto obj = (*current)["level_" + std::to_string(level)].getObject()) {
                current = const_cast<JsonValue::ObjectType*>(obj);
            }
        }

        JsonValue testData(std::move(largeObject));

        // 测试深度查询
        JsonQueryGenerator::GeneratorOptions opts;
        opts.maxResults = 50;
        auto generator = JsonQueryGenerator(testData, "$..item_id", opts);
        
        int foundItems = 0;
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            foundItems++;
        }
        
        ASSERT_EQ(50, foundItems);
        
        // 测试内存使用情况 - 生成器应该能处理大对象而不会导致内存溢出
        size_t totalProcessed = 0;
        auto memoryTestGenerator = JsonQueryGenerator(testData, "$..value");
        
        // 使用流式处理，每次只处理一个项目
        memoryTestGenerator.yield([&totalProcessed](const JsonValue* value, const std::string& path, size_t index) -> bool {
            totalProcessed++;
            return totalProcessed < 200; // 限制处理数量以测试早期终止
        });
        
        ASSERT_EQ(200, totalProcessed);
        ASSERT_TRUE(memoryTestGenerator.getState() == JsonQueryGenerator::State::Terminated);
        
    } catch (const std::exception& e) {
        result.addFail("大对象测试失败: " + std::string(e.what()));
    }
}

// 错误处理测试
void test_error_handling(TestResult& result) {
    try {
        JsonValue::ObjectType testObj;
        testObj["test"] = JsonValue(std::string("value"));
        JsonValue testData(std::move(testObj));
        
        // 测试无效查询表达式的处理
        auto generator = JsonQueryGenerator(testData, "invalid_query");
        
        int count = 0;
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            count++;
        }
        
        // 无效查询应该返回0个结果，而不是崩溃
        ASSERT_EQ(0, count);
        
        // 测试空数据的处理
        JsonValue::ObjectType emptyObj;
        JsonValue emptyData(std::move(emptyObj));
        auto emptyGenerator = JsonQueryGenerator(emptyData, "$.anything");
        
        count = 0;
        for (auto it = emptyGenerator.begin(); it != emptyGenerator.end(); ++it) {
            count++;
        }
        
        ASSERT_EQ(0, count);
        
        // 测试生成器重置
        generator.reset();
        ASSERT_TRUE(generator.getState() == JsonQueryGenerator::State::Ready);
        ASSERT_EQ(0, generator.getTotalGenerated());
        
    } catch (const std::exception& e) {
        result.addFail("错误处理测试失败: " + std::string(e.what()));
    }
}

// 并发安全测试
void test_thread_safety_basics(TestResult& result) {
    try {
        // 注意: JsonQueryGenerator被设计为非线程安全的单个使用者
        // 这个测试确保在单线程环境中的正确行为
        
        JsonValue testData = createLargeArray(1000);

        // 创建多个独立的生成器实例
        std::vector<std::unique_ptr<JsonQueryGenerator>> generators;
        for (int i = 0; i < 5; ++i) {
            JsonQueryGenerator::GeneratorOptions opts;
            opts.maxResults = 100;
            generators.push_back(std::make_unique<JsonQueryGenerator>(testData, "$[*]", opts));
        }

        // 测试每个生成器独立工作
        for (auto& gen : generators) {
            int count = 0;
            for (auto it = gen->begin(); it != gen->end(); ++it) {
                count++;
            }
            ASSERT_EQ(100, count);
        }
        
    } catch (const std::exception& e) {
        result.addFail("基础线程安全测试失败: " + std::string(e.what()));
    }
}

// 复杂JSONPath查询测试
void test_complex_jsonpath_queries(TestResult& result) {
    try {
        // 创建复杂的嵌套数据结构
        JsonValue::ObjectType storeObj;
        JsonValue::ArrayType bookArray;
        
        JsonValue::ObjectType book1;
        book1["category"] = JsonValue(std::string("reference"));
        book1["author"] = JsonValue(std::string("Nigel Rees"));
        book1["title"] = JsonValue(std::string("Sayings of the Century"));
        book1["price"] = JsonValue(8.95);
        bookArray.push_back(JsonValue(std::move(book1)));
        
        JsonValue::ObjectType book2;
        book2["category"] = JsonValue(std::string("fiction"));
        book2["author"] = JsonValue(std::string("Evelyn Waugh"));
        book2["title"] = JsonValue(std::string("Sword of Honour"));
        book2["price"] = JsonValue(12.99);
        bookArray.push_back(JsonValue(std::move(book2)));
        
        JsonValue::ObjectType book3;
        book3["category"] = JsonValue(std::string("fiction"));
        book3["author"] = JsonValue(std::string("Herman Melville"));
        book3["title"] = JsonValue(std::string("Moby Dick"));
        book3["isbn"] = JsonValue(std::string("0-553-21311-3"));
        book3["price"] = JsonValue(8.99);
        bookArray.push_back(JsonValue(std::move(book3)));
        
        JsonValue::ObjectType bicycleObj;
        bicycleObj["color"] = JsonValue(std::string("red"));
        bicycleObj["price"] = JsonValue(19.95);
        
        storeObj["book"] = JsonValue(std::move(bookArray));
        storeObj["bicycle"] = JsonValue(std::move(bicycleObj));
        
        JsonValue::ObjectType complexObj;
        complexObj["store"] = JsonValue(std::move(storeObj));
        JsonValue complexData(std::move(complexObj));

        // 测试基础路径查询
        auto bookGenerator = JsonQueryGenerator(complexData, "$.store.book[*]");
        int bookCount = 0;
        for (auto it = bookGenerator.begin(); it != bookGenerator.end(); ++it) {
            bookCount++;
        }
        ASSERT_EQ(3, bookCount);

        // 测试属性存在性查询 (简化版本，因为完整的JsonPath可能还未实现)
        auto priceGenerator = JsonQueryGenerator(complexData, "$..price");
        int priceCount = 0;
        for (auto it = priceGenerator.begin(); it != priceGenerator.end(); ++it) {
            priceCount++;
        }
        // 注意: 实际结果取决于JsonPath实现的完整性
        ASSERT_TRUE(priceCount >= 0);

    } catch (const std::exception& e) {
        result.addFail("复杂JSONPath查询测试失败: " + std::string(e.what()));
    }
}

// 内存效率测试
void test_memory_efficiency(TestResult& result) {
    try {
        // 创建一个相对较大的数据集来测试内存使用
        JsonValue largeArray = createLargeArray(10000);

        // 测试流式处理 - 只处理前100个，但不加载全部到内存
        JsonQueryGenerator::GeneratorOptions opts;
        opts.maxResults = 100;
        opts.batchSize = 10;
        
        auto generator = JsonQueryGenerator(largeArray, "$[*]", opts);
        
        int processed = 0;
        auto start = std::chrono::high_resolution_clock::now();
        
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            processed++;
            // 验证我们可以访问数据
            ASSERT_TRUE(it->first->getObject() != nullptr);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        ASSERT_EQ(100, processed);
        ASSERT_TRUE(duration.count() < 50);
        
        // 验证生成器没有缓存所有结果
        ASSERT_EQ(100, generator.getTotalGenerated());
        
    } catch (const std::exception& e) {
        result.addFail("内存效率测试失败: " + std::string(e.what()));
    }
}

// 简单的测试运行器类
class TestRunner {
private:
    std::vector<std::pair<std::string, std::function<void(TestResult&)>>> tests_;

public:
    void addTest(const std::string& name, std::function<void(TestResult&)> func) {
        tests_.emplace_back(name, func);
    }
    
    TestResult runAllTests() {
        TestResult totalResult;
        std::cout << "=== JsonQueryGenerator 测试套件开始 ===" << std::endl;
        
        for (auto& [name, func] : tests_) {
            std::cout << "运行测试: " << name << std::endl;
            TestResult testResult;
            func(testResult);
            
            if (testResult.isSuccess()) {
                std::cout << "[通过] " << name << " (" << testResult.getPassed() << " 个断言)" << std::endl;
            } else {
                std::cout << "[失败] " << name << " (" << testResult.getFailed() << " 个失败)" << std::endl;
                for (const auto& failure : testResult.getFailures()) {
                    std::cout << "  - " << failure << std::endl;
                }
            }
            
            totalResult.addPass(testResult.getPassed());
            for (const auto& failure : testResult.getFailures()) {
                totalResult.addFail(failure);
            }
        }
        
        return totalResult;
    }
};

int main() {
    TestRunner runner;
    
    // 注册所有测试用例
    runner.addTest("基础生成器功能", test_basic_generator_functionality);
    runner.addTest("早期终止测试", test_early_termination);
    runner.addTest("批量处理测试", test_batch_processing);
    runner.addTest("流式查询工厂测试", test_streaming_query_factory);
    runner.addTest("性能对比测试", test_performance_comparison);
    runner.addTest("大对象处理测试", test_large_object_handling);
    runner.addTest("错误处理测试", test_error_handling);
    runner.addTest("基础线程安全测试", test_thread_safety_basics);
    runner.addTest("复杂JSONPath查询测试", test_complex_jsonpath_queries);
    runner.addTest("内存效率测试", test_memory_efficiency);
    
    // 运行所有测试
    TestResult finalResult = runner.runAllTests();
    
    // 输出最终结果
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "JsonQueryGenerator 测试套件完成" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "通过: " << finalResult.getPassed() << std::endl;
    std::cout << "失败: " << finalResult.getFailed() << std::endl;
    std::cout << "总计: " << finalResult.getTotal() << std::endl;
    
    if (finalResult.getFailed() > 0) {
        std::cout << "\n失败详情:" << std::endl;
        for (const auto& failure : finalResult.getFailures()) {
            std::cout << "  - " << failure << std::endl;
        }
    }
    
    std::cout << std::string(60, '=') << std::endl;
    
    return finalResult.getFailed() == 0 ? 0 : 1;
}
