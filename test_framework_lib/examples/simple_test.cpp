#include <test_framework/test_framework.h>

// A basic test case
TEST(AdditionTest)
{
    int a = 1;
    int b = 2;
    ASSERT_EQ(a + b, 3);
    ASSERT_TRUE(a < b);
}

// Another test case to demonstrate string assertions
TEST(StringTest)
{
    std::string hello = "Hello";
    std::string world = "World";
    ASSERT_STREQ(hello.c_str(), "Hello");
    ASSERT_STRNE(hello.c_str(), world.c_str());
}

// The main function that runs all registered tests
int main(int argc, char* argv[])
{
    // The RUN_ALL_TESTS() macro expands to the code that
    // runs the tests and returns a status code.
    return RUN_ALL_TESTS();
}
