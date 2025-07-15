#include "../test_framework/test_framework.h"
#include <iostream>

int main() {
    std::cout << "=== JsonStruct Library Test Suite ===" << std::endl;
    std::cout << "Running tests using custom test framework..." << std::endl;
    std::cout << std::endl;
    
    TestFramework::TestSuite& suite = TestFramework::TestSuite::instance();
    int result = suite.runAll();
    
    std::cout << std::endl;
    std::cout << "=== Test Results Summary ===" << std::endl;
    
    if (result == 0) {
        std::cout << "✅ All tests PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some tests FAILED!" << std::endl;
        return 1;
    }
}
