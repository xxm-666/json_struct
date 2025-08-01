# JsonStruct 版本管理系统使用指南

JsonStruct 现在包含了完整的版本管理系统，提供运行时版本查询、API兼容性检查等功能。

## 功能概述

### 核心功能
- 🔍 **版本信息查询** - 获取当前库版本、构建日期、Git信息
- ✅ **兼容性检查** - 检查当前版本是否支持指定的API版本
- 📊 **版本比较** - 比较版本号大小关系
- 🏗️ **构建信息** - 自动集成Git提交、分支、构建时间
- 🛠️ **命令行工具** - 独立的版本信息查询工具

## API 使用

### 基本版本信息
```cpp
#include "version.h"
using namespace JsonStruct;

// 获取版本字符串 (例: "1.2.0-dev")
std::string version = Version::getVersionString();

// 获取详细版本信息
std::string detailed = Version::getDetailedVersionString();
// 输出: "JsonStruct v1.2.0-dev (built on 2025-07-18 06:33:23 UTC) [b23572d on dev]"

// 获取版本元组
auto [major, minor, patch] = Version::getVersionTuple();
std::cout << "Version: " << major << "." << minor << "." << patch << std::endl;
```

### 兼容性检查
```cpp
// 检查是否支持v1.0+ API
if (Version::isCompatible(1, 0)) {
    std::cout << "支持v1.0+ API" << std::endl;
}

// 检查是否支持v1.2+ API
if (Version::isCompatible(1, 2)) {
    // 使用新功能
    useAdvancedFeatures();
}

// 检查不兼容的版本
if (!Version::isCompatible(2, 0)) {
    std::cout << "不支持v2.0 API" << std::endl;
}
```

### 版本比较
```cpp
// 比较版本
int result = Version::compareVersion(1, 1, 5);
if (result > 0) {
    std::cout << "当前版本高于1.1.5" << std::endl;
} else if (result < 0) {
    std::cout << "当前版本低于1.1.5" << std::endl;
} else {
    std::cout << "当前版本等于1.1.5" << std::endl;
}
```

### 构建和Git信息
```cpp
// 获取构建信息
std::cout << "构建日期: " << Version::getBuildDate() << std::endl;
std::cout << "Git提交: " << Version::getGitCommit() << std::endl;
std::cout << "Git分支: " << Version::getGitBranch() << std::endl;

// 检查是否为发布版本
if (Version::isReleaseVersion()) {
    std::cout << "这是一个发布版本" << std::endl;
} else {
    std::cout << "这是一个开发版本" << std::endl;
}
```

## 命令行工具

### version_info 工具使用

```bash
# 显示简短版本
./version_info --version
# 输出: 1.2.0-dev

# 显示详细版本信息
./version_info --detailed  
# 输出: JsonStruct v1.2.0-dev (built on 2025-07-18 06:33:23 UTC) [b23572d on dev]

# JSON格式输出
./version_info --json
# 输出: {"version": "1.2.0-dev", "major": 1, ...}

# 兼容性检查
./version_info --check 1.0
# 输出: Version compatibility check: 1.2.0-dev with 1.0 -> Compatible

# 检查不兼容版本
./version_info --check 2.0
# 输出: Version compatibility check: 1.2.0-dev with 2.0 -> Incompatible
#       Reason: Major version mismatch (current: 1, required: 2)
```

## 实际应用场景

### 1. 应用启动日志
```cpp
int main() {
    std::cout << "启动应用程序: " << Version::getDetailedVersionString() << std::endl;
    
    // 检查依赖版本
    if (!Version::isCompatible(1, 0)) {
        std::cerr << "错误: 需要JsonStruct v1.0+" << std::endl;
        return 1;
    }
    
    // 正常启动
    return 0;
}
```

### 2. 功能可用性检查
```cpp
void processData() {
    auto [major, minor, patch] = Version::getVersionTuple();
    
    if (major >= 1 && minor >= 2) {
        // 使用v1.2+的高级功能
        useAdvancedJSONPathFeatures();
    } else {
        // 使用基础功能
        useBasicFeatures();
    }
}
```

### 3. API响应中包含版本信息
```cpp
std::string getApiInfo() {
    return R"({
        "api_version": ")" + Version::getVersionString() + R"(",
        "build_info": ")" + Version::getBuildDate() + R"(",
        "git_commit": ")" + Version::getGitCommit() + R"("
    })";
}
```

### 4. 调试信息输出
```cpp
void dumpDebugInfo() {
    std::cout << "=== Debug Information ===" << std::endl;
    std::cout << "Library Version: " << Version::getVersionString() << std::endl;
    std::cout << "Build Date: " << Version::getBuildDate() << std::endl;
    std::cout << "Git Commit: " << Version::getGitCommit() << std::endl;
    std::cout << "Git Branch: " << Version::getGitBranch() << std::endl;
}
```

## 构建集成

版本信息在CMake构建时自动生成，包括：
- 项目版本号（从CMakeLists.txt获取）
- Git提交哈希（短格式）
- Git分支名称
- 构建时间戳

无需手动维护版本信息，所有数据都在构建时自动注入。

## 语义化版本控制

JsonStruct 遵循 [语义化版本控制](https://semver.org/) 规范：
- **主版本号(MAJOR)**: 不兼容的API变更
- **次版本号(MINOR)**: 向后兼容的功能性新增  
- **修订号(PATCH)**: 向后兼容的问题修正

兼容性检查基于这个规则：当前版本与指定版本兼容，当且仅当主版本号相同且次版本号不低于指定版本。
