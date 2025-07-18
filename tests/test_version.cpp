#include <iostream>
#include <vector>
#include "version.h"

using namespace JsonStruct;

void test_version_basic() {
    std::cout << "=== Basic Version Information Test ===" << std::endl;
    
    std::cout << "Version String: " << Version::getVersionString() << std::endl;
    std::cout << "Detailed Version: " << Version::getDetailedVersionString() << std::endl;
    
    auto [major, minor, patch] = Version::getVersionTuple();
    std::cout << "Version Tuple: " << major << "." << minor << "." << patch << std::endl;
    
    std::cout << "Build Date: " << Version::getBuildDate() << std::endl;
    std::cout << "Git Commit: " << Version::getGitCommit() << std::endl;
    std::cout << "Git Branch: " << Version::getGitBranch() << std::endl;
    std::cout << "Is Release: " << (Version::isReleaseVersion() ? "Yes" : "No") << std::endl;
}

void test_version_compatibility() {
    std::cout << "\n=== Version Compatibility Test ===" << std::endl;
    
    // Test compatibility checks
    std::cout << "Compatible with 1.0: " << (Version::isCompatible(1, 0) ? "Yes" : "No") << std::endl;
    std::cout << "Compatible with 1.1: " << (Version::isCompatible(1, 1) ? "Yes" : "No") << std::endl;
    std::cout << "Compatible with 1.3: " << (Version::isCompatible(1, 3) ? "Yes" : "No") << std::endl;
    std::cout << "Compatible with 2.0: " << (Version::isCompatible(2, 0) ? "Yes" : "No") << std::endl;
}

void test_version_comparison() {
    std::cout << "\n=== Version Comparison Test ===" << std::endl;
    
    struct TestCase {
        int major, minor, patch;
        std::string description;
    };
    
    std::vector<TestCase> test_cases = {
        {1, 1, 0, "1.1.0"},
        {1, 2, 0, "1.2.0"},
        {1, 2, 1, "1.2.1"},
        {1, 3, 0, "1.3.0"},
        {2, 0, 0, "2.0.0"}
    };
    
    for (const auto& test_case : test_cases) {
        int result = Version::compareVersion(test_case.major, test_case.minor, test_case.patch);
        std::string comparison;
        if (result < 0) comparison = "lower than";
        else if (result > 0) comparison = "higher than";
        else comparison = "equal to";
        
        std::cout << "Current version " << comparison << " " << test_case.description << std::endl;
    }
}

void test_version_json() {
    std::cout << "\n=== 版本JSON输出测试 ===" << std::endl;
    std::cout << Version::toJson() << std::endl;
}

void test_version_macros() {
    std::cout << "\n=== 宏定义测试 ===" << std::endl;
    std::cout << "JSON_STRUCT_VERSION_STRING: " << JSON_STRUCT_VERSION_STRING << std::endl;
    std::cout << "JSON_STRUCT_VERSION_CHECK(1,0): " << (JSON_STRUCT_VERSION_CHECK(1, 0) ? "true" : "false") << std::endl;
    std::cout << "JSON_STRUCT_VERSION_CHECK(2,0): " << (JSON_STRUCT_VERSION_CHECK(2, 0) ? "true" : "false") << std::endl;
}

void demonstrate_usage() {
    std::cout << "\n=== 实际使用示例 ===" << std::endl;
    
    // 示例1：检查运行时版本兼容性
    std::cout << "示例1 - 运行时版本检查:" << std::endl;
    if (Version::isCompatible(1, 0)) {
        std::cout << "  ✅ 当前版本支持v1.0+ API" << std::endl;
    } else {
        std::cout << "  ❌ 当前版本不支持v1.0+ API" << std::endl;
    }
    
    // 示例2：版本信息记录
    std::cout << "\n示例2 - 应用程序启动日志:" << std::endl;
    std::cout << "  [INFO] 启动 JsonStruct 库 " << Version::getDetailedVersionString() << std::endl;
    
    // 示例3：调试信息
    std::cout << "\n示例3 - 调试信息输出:" << std::endl;
    auto [maj, min, pat] = Version::getVersionTuple();
    std::cout << "  库版本: " << maj << "." << min << "." << pat << std::endl;
    std::cout << "  构建信息: " << Version::getBuildDate() << " (" << Version::getGitCommit() << ")" << std::endl;
}

int main() {
    try {
        std::cout << "JsonStruct 版本管理系统测试\n" << std::endl;
        
        test_version_basic();
        test_version_compatibility();
        test_version_comparison();
        test_version_json();
        test_version_macros();
        demonstrate_usage();
        
        std::cout << "\n所有版本管理功能测试完成！" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "测试失败: " << e.what() << std::endl;
        return 1;
    }
}
