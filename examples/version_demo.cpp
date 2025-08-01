#include <iostream>
#include "../src/version.h"

using namespace JsonStruct;

/**
 * @brief 演示如何在实际应用中使用版本管理功能
 */
int main() {
    std::cout << "=== JsonStruct Version Management Demo ===" << std::endl;
    
    // 1. 应用启动时显示版本信息
    std::cout << "\n1. Application Startup:" << std::endl;
    std::cout << "Starting application with " << Version::getDetailedVersionString() << std::endl;
    
    // 2. 运行时版本检查
    std::cout << "\n2. Runtime Version Checks:" << std::endl;
    
    // 检查是否支持某个API版本
    if (Version::isCompatible(1, 0)) {
        std::cout << "[OK] v1.0+ API supported" << std::endl;
    }
    
    // 检查新功能可用性
    auto [major, minor, patch] = Version::getVersionTuple();
    if (major >= 1 && minor >= 2) {
        std::cout << "[OK] Advanced features available (v1.2+)" << std::endl;
    }
    
    // 3. 版本比较示例
    std::cout << "\n3. Version Comparison Examples:" << std::endl;
    
    struct RequiredVersion {
        int major, minor;
        const char* feature;
    };
    
    RequiredVersion features[] = {
        {1, 0, "Basic JSON parsing"},
        {1, 1, "Enhanced error handling"},
        {1, 2, "Advanced JSONPath features"},
        {1, 3, "Future feature (not available)"},
        {2, 0, "Major API redesign (not available)"}
    };
    
    for (const auto& feature : features) {
        bool available = Version::isCompatible(feature.major, feature.minor);
        std::cout << "  " << (available ? "[OK]" : "[NO]") << " " 
                  << feature.feature << " (requires v" 
                  << feature.major << "." << feature.minor << "+)" << std::endl;
    }
    
    // 4. 构建和部署信息
    std::cout << "\n4. Build and Deployment Info:" << std::endl;
    std::cout << "Build Date: " << Version::getBuildDate() << std::endl;
    std::cout << "Git Commit: " << Version::getGitCommit() << std::endl;
    std::cout << "Git Branch: " << Version::getGitBranch() << std::endl;
    std::cout << "Release Version: " << (Version::isReleaseVersion() ? "Yes" : "No (Development)") << std::endl;
    
    // 5. 程序信息演示
    std::cout << "\n5. Program Integration Example:" << std::endl;
    std::cout << "Library: " << Version::getVersionString() << std::endl;
    std::cout << "Compatible with older versions: " << (Version::isCompatible(1, 0) ? "Yes" : "No") << std::endl;
    
    return 0;
}
