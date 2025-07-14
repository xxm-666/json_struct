#include "json_filter.h"
#include "json_value.h"
#include "json_path.h"
#include <algorithm>
#include <sstream>
#include <cctype>
#include <unordered_set>

namespace JsonStruct {

// === QueryResult转换实现 ===

std::vector<JsonFilter::QueryResult> JsonFilter::QueryResult::fromJsonPathResult(const jsonpath::QueryResult& jpResult) {
    std::vector<JsonFilter::QueryResult> results;
    results.reserve(jpResult.size());
    
    for (size_t i = 0; i < jpResult.size(); ++i) {
        JsonFilter::QueryResult result;
        result.value = &(jpResult.values[i].get());
        result.path = jpResult.paths[i];
        result.depth = std::count(result.path.begin(), result.path.end(), '.') + 
                      std::count(result.path.begin(), result.path.end(), '[');
        
        // 简单的路径分解（可以进一步优化）
        // TODO: 可以使用json_path.h的Token信息进行更精确的分解
        
        results.push_back(result);
    }
    
    return results;
}

// === JsonFilter 实现 ===

JsonFilter::JsonFilter(const QueryOptions& options) : options_(options) {
}

void JsonFilter::setOptions(const QueryOptions& options) {
    options_ = options;
    if (!options_.enableCaching) {
        clearCache();
    }
}

const JsonFilter::QueryOptions& JsonFilter::getOptions() const noexcept {
    return options_;
}

void JsonFilter::clearCache() {
    queryCache_.clear();
}

bool JsonFilter::pathExists(const JsonValue& jsonValue, const std::string& expression) const {
    return selectFirst(jsonValue, expression) != nullptr;
}

const JsonValue* JsonFilter::selectFirst(const JsonValue& jsonValue, const std::string& expression) const {
    auto results = executeQuery(jsonValue, expression);
    return results.empty() ? nullptr : results[0].value;
}

std::vector<const JsonValue*> JsonFilter::selectAll(const JsonValue& jsonValue, const std::string& expression) const {
    auto results = executeQuery(jsonValue, expression);
    std::vector<const JsonValue*> values;
    values.reserve(results.size());
    
    for (const auto& result : results) {
        values.push_back(result.value);
    }
    
    return values;
}

std::vector<JsonValue> JsonFilter::selectValues(const JsonValue& jsonValue, const std::string& expression) const {
    auto results = executeQuery(jsonValue, expression);
    std::vector<JsonValue> values;
    values.reserve(results.size());
    
    for (const auto& result : results) {
        if (result.value) {
            values.push_back(*result.value);
        }
    }
    
    return values;
}

std::vector<JsonFilter::QueryResult> JsonFilter::query(const JsonValue& jsonValue, const std::string& expression) const {
    return executeQuery(jsonValue, expression);
}

std::vector<JsonFilter::QueryResult> JsonFilter::queryWithFilter(const JsonValue& jsonValue, const FilterFunction& filter) const {
    std::vector<QueryResult> results;
    std::function<void(const JsonValue&, const std::string&, size_t)> traverse;
    
    traverse = [&](const JsonValue& current, const std::string& currentPath, size_t depth) {
        if (matchesFilter(current, currentPath, filter)) {
            QueryResult result;
            result.value = &current;
            result.path = currentPath;
            result.depth = depth;
            // TODO: 解析 pathTokens
            results.push_back(result);
        }
        
        if (options_.maxResults > 0 && results.size() >= options_.maxResults) {
            return;
        }
        
        if (current.isObject()) {
            const auto* obj = current.getObject();
            if (obj) {
                for (const auto& [key, value] : *obj) {
                    std::string newPath = buildPath(currentPath, key);
                    traverse(value, newPath, depth + 1);
                    if (options_.maxResults > 0 && results.size() >= options_.maxResults) {
                        return;
                    }
                }
            }
        } else if (current.isArray()) {
            const auto* arr = current.getArray();
            if (arr) {
                for (size_t i = 0; i < arr->size(); ++i) {
                    std::string newPath = buildArrayPath(currentPath, i);
                    traverse((*arr)[i], newPath, depth + 1);
                    if (options_.maxResults > 0 && results.size() >= options_.maxResults) {
                        return;
                    }
                }
            }
        }
    };
    
    traverse(jsonValue, "$", 0);
    return results;
}

std::vector<JsonFilter::QueryResult> JsonFilter::queryWithRegex(const JsonValue& jsonValue, const std::string& pathPattern) const {
    std::regex pathRegex(pathPattern);
    
    auto filter = [&pathRegex](const JsonValue&, const std::string& path) {
        return std::regex_match(path, pathRegex);
    };
    
    return queryWithFilter(jsonValue, filter);
}

std::vector<std::vector<JsonFilter::QueryResult>> JsonFilter::batchQuery(
    const JsonValue& jsonValue, 
    const std::vector<std::string>& expressions) const {
    
    std::vector<std::vector<QueryResult>> results;
    results.reserve(expressions.size());
    
    for (const auto& expression : expressions) {
        results.push_back(executeQuery(jsonValue, expression));
    }
    
    return results;
}

std::vector<JsonValue> JsonFilter::transform(
    const std::vector<QueryResult>& results, 
    const TransformFunction& transform) const {
    
    std::vector<JsonValue> transformed;
    transformed.reserve(results.size());
    
    for (const auto& result : results) {
        if (result.value) {
            transformed.push_back(transform(*result.value, result.path));
        }
    }
    
    return transformed;
}

JsonFilter::QueryBuilder JsonFilter::from(const JsonValue& jsonValue) const {
    return QueryBuilder(*this, jsonValue);
}

// === 静态工厂方法 ===

JsonFilter JsonFilter::createDefault() {
    QueryOptions options;
    return JsonFilter(options);
}

JsonFilter JsonFilter::createHighPerformance() {
    QueryOptions options;
    options.enableCaching = true;
    options.stopOnFirstMatch = false; // 可能需要根据具体使用场景调整
    return JsonFilter(options);
}

JsonFilter JsonFilter::createStrict() {
    QueryOptions options;
    options.caseSensitive = true;
    options.allowWildcard = false;
    options.recursiveDescentEnabled = false;
    options.slicingEnabled = false;
    options.filterExpressionsEnabled = false;
    return JsonFilter(options);
}

// === 预定义过滤器实现 ===

JsonFilter::FilterFunction JsonFilter::Filters::byType(int typeValue) {
    return [typeValue](const JsonValue& value, const std::string&) {
        return static_cast<int>(value.type()) == typeValue;
    };
}

JsonFilter::FilterFunction JsonFilter::Filters::byString(const std::string& targetValue, bool caseSensitive) {
    return [targetValue, caseSensitive](const JsonValue& value, const std::string&) {
        if (!value.isString()) return false;
        
        auto strValue = value.getString();
        if (!strValue) return false;
        
        if (caseSensitive) {
            return std::string(*strValue) == targetValue;
        } else {
            std::string str1(*strValue), str2(targetValue);
            std::transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
            std::transform(str2.begin(), str2.end(), str2.begin(), ::tolower);
            return str1 == str2;
        }
    };
}

JsonFilter::FilterFunction JsonFilter::Filters::byNumber(double targetValue, double tolerance) {
    return [targetValue, tolerance](const JsonValue& value, const std::string&) {
        if (!value.isNumber()) return false;
        
        auto numValue = value.getNumber();
        if (!numValue) return false;
        
        return std::abs(*numValue - targetValue) <= tolerance;
    };
}

JsonFilter::FilterFunction JsonFilter::Filters::byNumberRange(double min, double max) {
    return [min, max](const JsonValue& value, const std::string&) {
        if (!value.isNumber()) return false;
        
        auto numValue = value.getNumber();
        if (!numValue) return false;
        
        return *numValue >= min && *numValue <= max;
    };
}

JsonFilter::FilterFunction JsonFilter::Filters::byDepth(size_t minDepth, size_t maxDepth) {
    return [minDepth, maxDepth](const JsonValue&, const std::string& path) {
        size_t depth = std::count(path.begin(), path.end(), '.') + std::count(path.begin(), path.end(), '[');
        return depth >= minDepth && depth <= maxDepth;
    };
}

JsonFilter::FilterFunction JsonFilter::Filters::byPathPattern(const std::string& pattern) {
    return [regex = std::regex(pattern)](const JsonValue&, const std::string& path) {
        return std::regex_match(path, regex);
    };
}

JsonFilter::FilterFunction JsonFilter::Filters::hasProperty(const std::string& property) {
    return [property](const JsonValue& value, const std::string&) {
        if (!value.isObject()) return false;
        return value.contains(property);
    };
}

JsonFilter::FilterFunction JsonFilter::Filters::arraySize(size_t minSize, size_t maxSize) {
    return [minSize, maxSize](const JsonValue& value, const std::string&) {
        if (!value.isArray()) return false;
        size_t size = value.size();
        return size >= minSize && size <= maxSize;
    };
}

JsonFilter::FilterFunction JsonFilter::Filters::isEmpty() {
    return [](const JsonValue& value, const std::string&) {
        return value.empty();
    };
}

JsonFilter::FilterFunction JsonFilter::Filters::isNotEmpty() {
    return [](const JsonValue& value, const std::string&) {
        return !value.empty();
    };
}

// === QueryBuilder 实现 ===

JsonFilter::QueryBuilder::QueryBuilder(const JsonFilter& filter, const JsonValue& jsonValue)
    : filter_(filter), jsonValue_(jsonValue) {
}

JsonFilter::QueryBuilder& JsonFilter::QueryBuilder::where(const std::string& expression) {
    expressions_.push_back(expression);
    return *this;
}

JsonFilter::QueryBuilder& JsonFilter::QueryBuilder::where(const FilterFunction& filter) {
    customFilters_.push_back(filter);
    return *this;
}

JsonFilter::QueryBuilder& JsonFilter::QueryBuilder::orderBy(const std::string& expression, bool ascending) {
    orderExpression_ = expression;
    orderAscending_ = ascending;
    return *this;
}

JsonFilter::QueryBuilder& JsonFilter::QueryBuilder::limit(size_t count) {
    limitCount_ = count;
    return *this;
}

JsonFilter::QueryBuilder& JsonFilter::QueryBuilder::skip(size_t count) {
    skipCount_ = count;
    return *this;
}

JsonFilter::QueryBuilder& JsonFilter::QueryBuilder::recursive() {
    recursiveMode_ = true;
    return *this;
}

JsonFilter::QueryBuilder& JsonFilter::QueryBuilder::shallow() {
    recursiveMode_ = false;
    return *this;
}

std::vector<JsonFilter::QueryResult> JsonFilter::QueryBuilder::execute() const {
    std::vector<QueryResult> results;
    
    // 如果没有任何查询条件，返回空结果
    if (expressions_.empty() && customFilters_.empty()) {
        return results;
    }
    
    // 开始时，将根对象作为初始结果集
    QueryResult initialResult;
    initialResult.value = &jsonValue_;
    initialResult.path = "$";
    initialResult.depth = 0;
    results.push_back(initialResult);
    
    // 依次应用每个表达式查询
    for (const auto& expression : expressions_) {
        std::vector<QueryResult> newResults;
        
        // 对当前结果集中的每个项目应用查询
        for (const auto& currentResult : results) {
            if (currentResult.value) {
                auto queryResults = filter_.executeQuery(*currentResult.value, expression);
                newResults.insert(newResults.end(), queryResults.begin(), queryResults.end());
            }
        }
        
        results = std::move(newResults);
    }
    
    // 依次应用每个自定义过滤器
    for (const auto& customFilter : customFilters_) {
        std::vector<QueryResult> filteredResults;
        
        // 对当前结果集应用过滤器
        for (const auto& result : results) {
            if (result.value && filter_.matchesFilter(*result.value, result.path, customFilter)) {
                filteredResults.push_back(result);
            }
        }
        
        results = std::move(filteredResults);
    }
    
    // 去重（基于值指针）
    std::unordered_set<const JsonValue*> seen;
    auto it = std::remove_if(results.begin(), results.end(), 
        [&seen](const QueryResult& result) {
            if (seen.count(result.value)) {
                return true; // 已存在，删除
            }
            seen.insert(result.value);
            return false; // 不存在，保留
        });
    results.erase(it, results.end());
    
    // 排序（如果指定了排序表达式）
    if (!orderExpression_.empty()) {
        // TODO: 实现基于表达式的排序
        // 这里需要根据 orderExpression_ 来排序结果
    }
    
    // 跳过指定数量的结果
    if (skipCount_ > 0 && skipCount_ < results.size()) {
        results.erase(results.begin(), results.begin() + skipCount_);
    }
    
    // 限制结果数量
    if (limitCount_ > 0 && results.size() > limitCount_) {
        results.resize(limitCount_);
    }
    
    return results;
}

std::optional<JsonFilter::QueryResult> JsonFilter::QueryBuilder::first() const {
    auto results = execute();
    return results.empty() ? std::nullopt : std::optional<QueryResult>(results[0]);
}

std::vector<JsonValue> JsonFilter::QueryBuilder::values() const {
    auto results = execute();
    std::vector<JsonValue> values;
    values.reserve(results.size());
    
    for (const auto& result : results) {
        if (result.value) {
            values.push_back(*result.value);
        }
    }
    
    return values;
}

size_t JsonFilter::QueryBuilder::count() const {
    return execute().size();
}

bool JsonFilter::QueryBuilder::any() const {
    return !execute().empty();
}

bool JsonFilter::QueryBuilder::all(const FilterFunction& predicate) const {
    auto results = execute();
    return std::all_of(results.begin(), results.end(), 
        [&predicate](const QueryResult& result) {
            return result.value && predicate(*result.value, result.path);
        });
}

// === 内部实现方法 ===

std::vector<JsonFilter::QueryResult> JsonFilter::executeQuery(
    const JsonValue& jsonValue, 
    const std::string& expression,
    FilterStrategy strategy) const {
    
    // 检查缓存
    if (options_.enableCaching) {
        std::string cacheKey = buildCacheKey(expression, strategy);
        std::vector<QueryResult> cachedResult;
        if (getCachedResult(cacheKey, cachedResult)) {
            return cachedResult;
        }
    }
    
    std::vector<QueryResult> results;
    
    switch (strategy) {
        case FilterStrategy::JSONPath: {
            // 使用统一的JSONPath引擎
            results = executeJsonPathUnified(jsonValue, expression);
            break;
        }
        default:
            // 其他策略的实现保持不变
            break;
    }
    
    // 缓存结果
    if (options_.enableCaching && !results.empty()) {
        std::string cacheKey = buildCacheKey(expression, strategy);
        setCachedResult(cacheKey, results);
    }
    
    return results;
}

std::vector<JsonFilter::QueryResult> JsonFilter::executeJsonPathUnified(
    const JsonValue& jsonValue, 
    const std::string& expression) const {
    
    try {
        // 使用json_path.h的JsonPath类
        jsonpath::JsonPath jsonPath(expression);
        auto jpResult = jsonPath.evaluate(jsonValue);
        
        // 转换为JsonFilter::QueryResult格式
        return QueryResult::fromJsonPathResult(jpResult);
        
    } catch (const jsonpath::JsonPathException& e) {
        // JSONPath解析错误，返回空结果
        return std::vector<QueryResult>();
    } catch (const std::exception& e) {
        // 其他错误，返回空结果
        return std::vector<QueryResult>();
    }
}

// === 缓存管理实现 ===

std::string JsonFilter::buildCacheKey(const std::string& expression, FilterStrategy strategy) const {
    return std::to_string(static_cast<int>(strategy)) + ":" + expression;
}

bool JsonFilter::getCachedResult(const std::string& cacheKey, std::vector<QueryResult>& result) const {
    auto it = queryCache_.find(cacheKey);
    if (it != queryCache_.end()) {
        result = it->second;
        return true;
    }
    return false;
}

void JsonFilter::setCachedResult(const std::string& cacheKey, const std::vector<QueryResult>& result) const {
    queryCache_[cacheKey] = result;
}

// === 工具方法实现 ===

bool JsonFilter::matchesFilter(const JsonValue& value, const std::string& path, const FilterFunction& filter) const {
    return filter(value, path);
}

std::string JsonFilter::buildPath(const std::string& basePath, const std::string& key) const {
    if (basePath == "$") {
        return "$." + key;
    }
    return basePath + "." + key;
}

std::string JsonFilter::buildArrayPath(const std::string& basePath, size_t index) const {
    return basePath + "[" + std::to_string(index) + "]";
}

bool JsonFilter::isValidArrayIndex(const std::string& token) const {
    if (token.length() < 3 || token[0] != '[' || token.back() != ']') {
        return false;
    }
    
    std::string indexStr = token.substr(1, token.length() - 2);
    return std::all_of(indexStr.begin(), indexStr.end(), ::isdigit);
}

size_t JsonFilter::parseArrayIndex(const std::string& token) const {
    std::string indexStr = token.substr(1, token.length() - 2);
    return std::stoull(indexStr);
}

} // namespace JsonStruct
