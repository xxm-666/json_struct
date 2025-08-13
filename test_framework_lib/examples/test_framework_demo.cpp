#include <test_framework/test_framework.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <climits>
// Demonstration of the enhanced test framework features

TEST(Enhanced_BasicAssertions) {
    ASSERT_TRUE(true);
    ASSERT_FALSE(false);
    ASSERT_EQ(42, 42);
    ASSERT_NE(42, 43);
    ASSERT_LT(1, 2);
    ASSERT_LE(2, 2);
    ASSERT_GT(3, 2);
    ASSERT_GE(3, 3);
    ASSERT_NEAR(3.14, 3.141, 0.01);
}

TEST(Enhanced_StringAssertions) {
    std::string hello = "Hello, World!";
    ASSERT_STREQ(hello, "Hello, World!");
    ASSERT_STRNE(hello, "Goodbye");
    ASSERT_CONTAINS(hello, "World");
    ASSERT_NOT_CONTAINS(hello, "xyz");
    ASSERT_STARTS_WITH(hello, "Hello");
    ASSERT_ENDS_WITH(hello, "World!");
    ASSERT_MATCHES_REGEX("test123", "test\\d+");
}

TEST(Enhanced_ContainerAssertions) {
    std::vector<int> empty_vec;
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    ASSERT_EMPTY(empty_vec);
    ASSERT_NOT_EMPTY(numbers);
    ASSERT_SIZE_EQ(numbers, 5);
}

TEST(Enhanced_ExceptionAssertions) {
    ASSERT_THROWS(throw std::runtime_error("test"), std::runtime_error);
    ASSERT_NO_THROW(int x = 42);
    ASSERT_ANY_THROW(throw std::logic_error("any exception"));
}

TEST(Enhanced_PointerAssertions) {
    int* null_ptr = nullptr;
    int value = 42;
    int* valid_ptr = &value;
    
    ASSERT_NULL(null_ptr);
    ASSERT_NOT_NULL(valid_ptr);
}

TEST_WITH_TAGS(Enhanced_TaggedTest, "demo", "enhanced") {
    ASSERT_TRUE(true);
}

TEST_SKIP(Enhanced_SkippedTest, "This test is intentionally skipped for demonstration")

TEST(Enhanced_ConditionalSkip) {
    bool should_skip = false;  // Could be based on environment, etc.
    if (should_skip) {
        SKIP_TEST("Skipping based on condition");
    }
    ASSERT_TRUE(true);  // This should be reached
}

// Test that demonstrates failure
TEST(Enhanced_IntentionalFailure) {
    // This test will fail to show how failures are displayed
    ASSERT_EQ(1, 2);  // This will fail
    ASSERT_TRUE(false); // This should not be reached due to early return
}


// 边界值测试
TEST(Enhanced_BoundaryValues) {
    ASSERT_EQ(INT_MAX, std::numeric_limits<int>::max());
    ASSERT_EQ(INT_MIN, std::numeric_limits<int>::min());
    ASSERT_STREQ("", std::string());
    ASSERT_SIZE_EQ(std::string(), 0);
}

// 多种容器类型
TEST(Enhanced_ContainerTypes) {
    std::set<int> s = {1, 2, 3};
    ASSERT_NOT_EMPTY(s);
    ASSERT_SIZE_EQ(s, 3);
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}};
    ASSERT_EQ(m["a"], 1);
    ASSERT_SIZE_EQ(m, 2);
}

// 多断言连续通过和失败
TEST(Enhanced_MultiAssertions) {
    ASSERT_TRUE(true);
    ASSERT_FALSE(false);
    ASSERT_EQ(10, 10);
    ASSERT_NE(10, 11);
    ASSERT_LT(1, 2);
    ASSERT_GT(2, 1);
    // 故意失败
    ASSERT_EQ(5, 6);
    ASSERT_TRUE(false);
}

// 复杂对象比较
struct Point {
    int x, y;
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
};
TEST(Enhanced_ComplexObject) {
    Point p1{1,2}, p2{1,2}, p3{2,3};
    ASSERT_TRUE(p1 == p2);
    ASSERT_FALSE(p1 == p3);
}

// context set/get 测试
class DemoContext : public TestFramework::TestContext {
public:
    void setUp() override { set("foo", 123); }
};
TEST(Enhanced_ContextSetGet) {
    DemoContext ctx;
    ctx.setUp();
    ASSERT_EQ(ctx.get<int>("foo"), 123);
    ctx.set("bar", std::string("hello"));
    ASSERT_STREQ(ctx.getString("bar"), "hello");
}

// 正则表达式异常情况
TEST(Enhanced_RegexInvalid) {
    ASSERT_MATCHES_REGEX("abc", "["); // invalid regex, should fail
}

int main() {
    TestFramework::TestConfig config;
    config.verbose = true;
    config.timing = true;
    config.stopOnFirstFailure = false;  // Continue even after failures
    
    SET_TEST_CONFIG(config);
    
    std::cout << "=== Enhanced Test Framework Demo ===" << std::endl;
    std::cout << "Demonstrating new features:" << std::endl;
    std::cout << "- Enhanced assertions (strings, containers, exceptions)" << std::endl;
    std::cout << "- Test timing" << std::endl;
    std::cout << "- Test tags" << std::endl;
    std::cout << "- Test skipping" << std::endl;
    std::cout << "- Better error reporting" << std::endl;
    std::cout << std::endl;
    
    return RUN_ALL_TESTS();
}
