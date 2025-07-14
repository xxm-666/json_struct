#pragma once

#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <unordered_map>
#include <regex>
#include <memory>

// 引入统一的JSONPath引擎
#include "json_path.h"

// Forward declaration to avoid circular dependency
namespace JsonStruct {
    class JsonValue;
}

namespace JsonStruct {

/**
 * @brief JsonFilter - 专门用于 JSON 数据过滤和查询的类
 * 
 * 这个类提供了强大的 JSON 数据过滤功能，支持：
 * - JSONPath 查询 (RFC 9535)
 * - 自定义过滤器函数
 * - 链式查询操作
 * - 批量查询和转换
 * 
 * 设计原则：
 * - 单一职责：专门负责数据过滤和查询
 * - 可扩展：支持自定义过滤器和查询策略
 * - 高性能：优化的查询算法和缓存机制
 * - 易用性：提供简洁的 API 和链式调用
 */
class JsonFilter {
public:
    // 查询选项
    struct QueryOptions {
        bool caseSensitive = true;          // 大小写敏感
        bool allowWildcard = true;          // 允许通配符
        bool enableCaching = false;         // 启用查询缓存
        size_t maxResults = 0;              // 最大结果数量 (0 = 无限制)
        bool stopOnFirstMatch = false;      // 找到第一个匹配就停止
        bool includeArrayIndices = true;    // 在结果中包含数组索引信息
        
        // 高级选项
        bool recursiveDescentEnabled = true; // 启用递归下降 (..)
        bool slicingEnabled = true;          // 启用数组切片 ([start:end:step])
        bool filterExpressionsEnabled = true; // 启用过滤表达式 ([?(@.price < 10)])
    };

    // 查询结果
    struct QueryResult {
        const JsonValue* value = nullptr;   // 指向查询到的值
        std::string path;                   // 完整的 JSONPath
        std::vector<std::string> pathTokens; // 路径分解
        size_t depth = 0;                   // 嵌套深度
        
        // 附加信息
        bool isArrayElement = false;        // 是否为数组元素
        size_t arrayIndex = 0;              // 如果是数组元素，其索引
        const JsonValue* parent = nullptr;  // 父节点指针
        
        // 构造函数
        QueryResult() = default;
        QueryResult(const JsonValue* val, const std::string& p, size_t d = 0)
            : value(val), path(p), depth(d) {}
            
        // 从jsonpath::QueryResult转换的便捷构造函数
        static std::vector<QueryResult> fromJsonPathResult(const jsonpath::QueryResult& jpResult);
        
        bool isValid() const noexcept { return value != nullptr; }
    };

    // 自定义过滤器函数类型
    using FilterFunction = std::function<bool(const JsonValue&, const std::string& path)>;
    using TransformFunction = std::function<JsonValue(const JsonValue&, const std::string& path)>;

    // 过滤器策略枚举
    enum class FilterStrategy {
        JSONPath,           // 使用 JSONPath 表达式
        CustomFunction,     // 使用自定义函数
        Regex,             // 使用正则表达式匹配路径
        Composite          // 组合多种策略
    };

public:
    // 构造函数
    explicit JsonFilter(const QueryOptions& options = {});
    
    // 禁用拷贝，允许移动
    JsonFilter(const JsonFilter&) = delete;
    JsonFilter& operator=(const JsonFilter&) = delete;
    JsonFilter(JsonFilter&&) = default;
    JsonFilter& operator=(JsonFilter&&) = default;

    // === 基础查询方法 ===
    
    /**
     * @brief 检查指定路径是否存在
     * @param jsonValue 要查询的 JSON 对象
     * @param expression JSONPath 表达式
     * @return 是否存在匹配的路径
     */
    bool pathExists(const JsonValue& jsonValue, const std::string& expression) const;
    
    /**
     * @brief 选择第一个匹配的值
     * @param jsonValue 要查询的 JSON 对象
     * @param expression JSONPath 表达式
     * @return 第一个匹配的值的指针，如果没有匹配则返回 nullptr
     */
    const JsonValue* selectFirst(const JsonValue& jsonValue, const std::string& expression) const;
    
    /**
     * @brief 选择所有匹配的值
     * @param jsonValue 要查询的 JSON 对象
     * @param expression JSONPath 表达式
     * @return 所有匹配值的指针向量
     */
    std::vector<const JsonValue*> selectAll(const JsonValue& jsonValue, const std::string& expression) const;
    
    /**
     * @brief 选择所有匹配的值（返回副本）
     * @param jsonValue 要查询的 JSON 对象
     * @param expression JSONPath 表达式
     * @return 所有匹配值的副本向量
     */
    std::vector<JsonValue> selectValues(const JsonValue& jsonValue, const std::string& expression) const;

    // === 高级查询方法 ===
    
    /**
     * @brief 执行详细查询，返回包含路径信息的结果
     * @param jsonValue 要查询的 JSON 对象
     * @param expression JSONPath 表达式
     * @return 详细的查询结果
     */
    std::vector<QueryResult> query(const JsonValue& jsonValue, const std::string& expression) const;
    
    /**
     * @brief 使用自定义过滤器函数进行查询
     * @param jsonValue 要查询的 JSON 对象
     * @param filter 自定义过滤器函数
     * @return 匹配的查询结果
     */
    std::vector<QueryResult> queryWithFilter(const JsonValue& jsonValue, const FilterFunction& filter) const;
    
    /**
     * @brief 使用正则表达式匹配路径进行查询
     * @param jsonValue 要查询的 JSON 对象
     * @param pathPattern 路径的正则表达式模式
     * @return 匹配的查询结果
     */
    std::vector<QueryResult> queryWithRegex(const JsonValue& jsonValue, const std::string& pathPattern) const;

    // === 链式查询方法 ===
    
    /**
     * @brief 创建链式查询构建器
     * @param jsonValue 要查询的 JSON 对象
     * @return 链式查询构建器
     */
    class QueryBuilder;
    QueryBuilder from(const JsonValue& jsonValue) const;

    // === 批量操作方法 ===
    
    /**
     * @brief 批量查询多个表达式
     * @param jsonValue 要查询的 JSON 对象
     * @param expressions JSONPath 表达式列表
     * @return 每个表达式对应的查询结果
     */
    std::vector<std::vector<QueryResult>> batchQuery(
        const JsonValue& jsonValue, 
        const std::vector<std::string>& expressions) const;
    
    /**
     * @brief 对查询结果应用转换函数
     * @param results 查询结果
     * @param transform 转换函数
     * @return 转换后的值
     */
    std::vector<JsonValue> transform(
        const std::vector<QueryResult>& results, 
        const TransformFunction& transform) const;

    // === 配置方法 ===
    
    /**
     * @brief 设置查询选项
     * @param options 新的查询选项
     */
    void setOptions(const QueryOptions& options);
    
    /**
     * @brief 获取当前查询选项
     * @return 当前的查询选项
     */
    const QueryOptions& getOptions() const noexcept;
    
    /**
     * @brief 清除查询缓存
     */
    void clearCache();

    // === 静态便利方法 ===
    
    /**
     * @brief 创建默认的过滤器实例
     * @return 默认配置的过滤器
     */
    static JsonFilter createDefault();
    
    /**
     * @brief 创建高性能配置的过滤器实例
     * @return 高性能配置的过滤器
     */
    static JsonFilter createHighPerformance();
    
    /**
     * @brief 创建严格模式的过滤器实例
     * @return 严格模式配置的过滤器
     */
    static JsonFilter createStrict();

    // === 预定义过滤器 ===
    
    /**
     * @brief 预定义的常用过滤器函数
     */
    struct Filters {
        // 基于类型的过滤器 - 使用 int 来避免前向声明问题
        static FilterFunction byType(int type);  // 使用 JsonValue::Type 的整数值
        static FilterFunction byString(const std::string& value, bool caseSensitive = true);
        static FilterFunction byNumber(double value, double tolerance = 1e-9);
        static FilterFunction byNumberRange(double min, double max);
        
        // 基于路径的过滤器
        static FilterFunction byDepth(size_t minDepth, size_t maxDepth = SIZE_MAX);
        static FilterFunction byPathPattern(const std::string& pattern);
        
        // 基于值的过滤器
        static FilterFunction hasProperty(const std::string& property);
        static FilterFunction arraySize(size_t minSize, size_t maxSize = SIZE_MAX);
        static FilterFunction isEmpty();
        static FilterFunction isNotEmpty();
    };

    // === 链式查询构建器 ===
    
    /**
     * @brief 链式查询构建器，支持流式 API
     */
    class QueryBuilder {
    public:
        explicit QueryBuilder(const JsonFilter& filter, const JsonValue& jsonValue);
        
        // 链式方法
        QueryBuilder& where(const std::string& expression);
        QueryBuilder& where(const FilterFunction& filter);
        QueryBuilder& orderBy(const std::string& expression, bool ascending = true);
        QueryBuilder& limit(size_t count);
        QueryBuilder& skip(size_t count);
        QueryBuilder& recursive();
        QueryBuilder& shallow();
        
        // 执行方法
        std::vector<QueryResult> execute() const;
        std::optional<QueryResult> first() const;
        std::vector<JsonValue> values() const;
        
        // 聚合方法
        size_t count() const;
        bool any() const;
        bool all(const FilterFunction& predicate) const;
        
    private:
        const JsonFilter& filter_;
        const JsonValue& jsonValue_;
        std::vector<std::string> expressions_;
        std::vector<FilterFunction> customFilters_;
        std::string orderExpression_;
        bool orderAscending_ = true;
        size_t limitCount_ = 0;
        size_t skipCount_ = 0;
        bool recursiveMode_ = false;
    };

private:
    // 查询选项
    QueryOptions options_;
    
    // 缓存相关
    mutable std::unordered_map<std::string, std::vector<QueryResult>> queryCache_;
    
    // 内部实现方法
    std::vector<QueryResult> executeQuery(
        const JsonValue& jsonValue, 
        const std::string& expression,
        FilterStrategy strategy = FilterStrategy::JSONPath) const;
    
    // 统一的JSONPath执行引擎（使用json_path.h）
    std::vector<QueryResult> executeJsonPathUnified(
        const JsonValue& jsonValue, 
        const std::string& expression) const;
    
    // 工具方法
    bool matchesFilter(const JsonValue& value, const std::string& path, const FilterFunction& filter) const;
    std::string buildPath(const std::string& basePath, const std::string& key) const;
    std::string buildArrayPath(const std::string& basePath, size_t index) const;
    bool isValidArrayIndex(const std::string& token) const;
    size_t parseArrayIndex(const std::string& token) const;
    
    // 缓存管理
    std::string buildCacheKey(const std::string& expression, FilterStrategy strategy) const;
    bool getCachedResult(const std::string& cacheKey, std::vector<QueryResult>& result) const;
    void setCachedResult(const std::string& cacheKey, const std::vector<QueryResult>& result) const;
};

// === 便利函数 ===

/**
 * @brief 全局便利函数，用于快速查询
 */
namespace query {
    
    // 快速查询函数
    inline bool exists(const JsonValue& json, const std::string& path) {
        return JsonFilter::createDefault().pathExists(json, path);
    }
    
    inline const JsonValue* first(const JsonValue& json, const std::string& path) {
        return JsonFilter::createDefault().selectFirst(json, path);
    }
    
    inline std::vector<const JsonValue*> all(const JsonValue& json, const std::string& path) {
        return JsonFilter::createDefault().selectAll(json, path);
    }
    
    inline std::vector<JsonValue> values(const JsonValue& json, const std::string& path) {
        return JsonFilter::createDefault().selectValues(json, path);
    }
    
    // 链式查询入口
    inline JsonFilter::QueryBuilder from(const JsonValue& json) {
        static JsonFilter defaultFilter = JsonFilter::createDefault();
        return defaultFilter.from(json);
    }
}

// 类型便利函数，避免前向声明问题
namespace filter_types {
    constexpr int Null = 0;
    constexpr int Bool = 1;
    constexpr int Number = 2;
    constexpr int String = 3;
    constexpr int Array = 4;
    constexpr int Object = 5;
}

} // namespace JsonStruct
