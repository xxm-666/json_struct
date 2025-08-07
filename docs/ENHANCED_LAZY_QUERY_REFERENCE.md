# Enhanced Lazy Query Generator 快速参考

## 基本用法

```cpp
#include "enhanced_query_factory.h"

// 1. 创建查询生成器
auto gen = EnhancedQueryFactory::createGenerator(filter, data, "$.path.to.data");

// 2. 启用缓存（可选）
gen.enableCache(true);

// 3. 处理结果
while (gen.hasNext()) {
    auto result = gen.next();
    // 使用 result.value 和 result.path
}
```

## JSONPath 语法速查

| 语法 | 描述 | 示例 |
|------|------|------|
| `$` | 根节点 | `$.root` |
| `.property` | 属性访问 | `$.user.name` |
| `['property']` | 属性访问（带特殊字符） | `$['user name']` |
| `[index]` | 数组索引 | `$.users[0]` |
| `[-index]` | 负索引 | `$.users[-1]` |
| `[*]` | 通配符 | `$.users[*].name` |
| `[start:end]` | 数组切片 | `$.users[1:4]` |
| `[start:end:step]` | 带步长切片 | `$.users[::2]` |
| `..` | 递归下降 | `$..name` |
| `?()` | 过滤器 | `$.users[?(@.age > 18)]` |
| `,` | Union操作 | `$.name,$.age` |

## 过滤器表达式

| 操作符 | 描述 | 示例 |
|--------|------|------|
| `==` | 等于 | `@.type == 'admin'` |
| `!=` | 不等于 | `@.status != 'inactive'` |
| `>` | 大于 | `@.price > 100` |
| `>=` | 大于等于 | `@.age >= 18` |
| `<` | 小于 | `@.score < 60` |
| `<=` | 小于等于 | `@.priority <= 5` |
| `&&` | 逻辑与 | `@.age > 18 && @.verified` |
| `\|\|` | 逻辑或 | `@.type == 'vip' \|\| @.score > 90` |

## 性能监控 API

```cpp
// 获取统计信息
int frames = gen.getFramesProcessed();      // 处理的帧数
int results = gen.getResultsGenerated();    // 生成的结果数
double efficiency = gen.getEfficiencyRatio(); // 效率比例

// 缓存统计
bool enabled = gen.isCacheEnabled();        // 缓存是否启用
double hitRatio = gen.getCacheHitRatio();   // 缓存命中率
int cacheSize = gen.getCacheSize();         // 缓存条目数
```

## 缓存管理

```cpp
gen.enableCache(true);     // 启用缓存
gen.enableCache(false);    // 禁用缓存
gen.clearCache();          // 清理缓存
gen.reset();               // 重置（保留缓存）
```

## 常用查询模式

### 1. 简单属性查询
```cpp
"$.user.profile.name"                    // 嵌套属性
"$['user data']['full name']"            // 带空格的属性
```

### 2. 数组操作
```cpp
"$.items[0]"                            // 第一个元素
"$.items[-1]"                           // 最后一个元素
"$.items[1:4]"                          // 切片
"$.items[*].name"                       // 所有元素的name属性
```

### 3. 条件过滤
```cpp
"$.products[?(@.price > 50)]"           // 价格筛选
"$.users[?(@.active && @.verified)]"    // 多条件
"$.books[?(@.category == 'fiction')]"   // 字符串匹配
```

### 4. 复合查询
```cpp
"$.store..book[?(@.price < 20)].title"  // 递归+过滤
"$.users[*].name,$.users[*].email"      // Union查询
"$..products[?(@.sale)][0:5]"           // 递归+过滤+切片
```

## 错误处理模式

```cpp
try {
    auto gen = EnhancedQueryFactory::createGenerator(filter, data, query);
    
    while (gen.hasNext()) {
        try {
            auto result = gen.next();
            processResult(result);
        } catch (const std::exception& e) {
            // 处理单个结果错误
            logError("Result processing error", e.what());
            continue;
        }
    }
} catch (const std::exception& e) {
    // 处理查询创建错误
    logError("Query creation error", e.what());
}
```

## 性能优化建议

### ✅ 推荐做法
- 使用具体路径而非宽泛递归
- 重复查询时启用缓存
- 分批处理大量结果
- 及时处理结果，避免全部存储

### ❌ 避免做法
- 过度使用 `$..` 递归查询
- 一次性查询时启用缓存
- 在内存中存储所有结果
- 忽略异常处理

## 调试技巧

```cpp
// 1. 分步调试
int count = 0;
while (gen.hasNext() && count < 10) {
    auto result = gen.next();
    std::cout << "Result " << count << ": " << result.path 
              << " = " << result.value->toString() << std::endl;
    count++;
}

// 2. 性能分析
auto start = std::chrono::steady_clock::now();
processAllResults(gen);
auto duration = getDuration(start);
std::cout << "Time: " << duration << "ms, "
          << "Efficiency: " << gen.getEfficiencyRatio() << "%" << std::endl;

// 3. 缓存分析
if (gen.isCacheEnabled()) {
    std::cout << "Cache hit ratio: " << gen.getCacheHitRatio() << "%" << std::endl;
}
```

## 常见问题 FAQ

**Q: 查询返回空结果？**
- 检查JSONPath语法
- 验证数据结构
- 使用简单查询测试

**Q: 性能较慢？**
- 启用缓存
- 避免复杂递归
- 检查查询复杂度

**Q: 内存使用过高？**
- 分批处理结果
- 清理不需要的缓存
- 检查数据规模

**Q: 缓存命中率低？**
- 确保查询完全相同
- 检查是否调用了 `reset()`
- 验证缓存启用状态
