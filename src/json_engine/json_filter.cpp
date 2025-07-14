#include "json_filter.h"
#include "json_value.h"
#include "json_path.h"
#include <algorithm>
#include <sstream>
#include <cctype>
#include <unordered_set>
#include <stdexcept>

namespace JsonStruct {

// === QueryResult conversion implementation ===

std::vector<JsonFilter::QueryResult> JsonFilter::QueryResult::fromJsonPathResult(const jsonpath::QueryResult& jpResult) {
    std::vector<JsonFilter::QueryResult> results;
    results.reserve(jpResult.size());
    
    for (size_t i = 0; i < jpResult.size(); ++i) {
        JsonFilter::QueryResult result;
        result.value = &(jpResult.values[i].get());
        result.path = jpResult.paths[i];
        result.depth = std::count(result.path.begin(), result.path.end(), '.') + 
                      std::count(result.path.begin(), result.path.end(), '[');
        
        // Simple path decomposition (can be further optimized)
        // TODO: Use Token info from json_path.h for more precise decomposition
        
        results.push_back(result);
    }
    
    return results;
}

// === JsonFilter implementation ===

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
            // TODO: Parse pathTokens
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

// === Static factory methods ===

JsonFilter JsonFilter::createDefault() {
    QueryOptions options;
    return JsonFilter(options);
}

JsonFilter JsonFilter::createHighPerformance() {
    QueryOptions options;
    options.enableCaching = true;
    options.stopOnFirstMatch = false; // May need adjustment based on specific use case
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

// === Predefined filter implementations ===

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

// === QueryBuilder implementation ===

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

JsonFilter::QueryBuilder& JsonFilter::QueryBuilder::groupBy(const std::string& expression) {
    groupByExpression_ = expression;
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
    
    // If there are no query conditions, return empty result
    if (expressions_.empty() && customFilters_.empty()) {
        return results;
    }
    
    // Start with the root object as the initial result set
    QueryResult initialResult;
    initialResult.value = &jsonValue_;
    initialResult.path = "$";
    initialResult.depth = 0;
    results.push_back(initialResult);
    
    // Apply each query expression in order
    for (const auto& expression : expressions_) {
        std::vector<QueryResult> newResults;
        
        // Apply query to each item in the current result set
        for (const auto& currentResult : results) {
            if (currentResult.value) {
                auto queryResults = filter_.executeQuery(*currentResult.value, expression);
                newResults.insert(newResults.end(), queryResults.begin(), queryResults.end());
            }
        }
        
        results = std::move(newResults);
    }
    
    // Apply each custom filter in order
    for (const auto& customFilter : customFilters_) {
        std::vector<QueryResult> filteredResults;
        
        // Apply filter to the current result set
        for (const auto& result : results) {
            if (result.value && filter_.matchesFilter(*result.value, result.path, customFilter)) {
                filteredResults.push_back(result);
            }
        }
        
        results = std::move(filteredResults);
    }
    
    // Remove duplicates (based on value pointer)
    std::unordered_set<const JsonValue*> seen;
    auto it = std::remove_if(results.begin(), results.end(), 
        [&seen](const QueryResult& result) {
            if (seen.count(result.value)) {
                return true; // Already exists, remove
            }
            seen.insert(result.value);
            return false; // Does not exist, keep
        });
    results.erase(it, results.end());
    
    // Sort (if a sort expression is specified)
    if (!orderExpression_.empty()) {
        std::sort(results.begin(), results.end(), [&](const QueryResult& a, const QueryResult& b) {
            const JsonValue* va = nullptr;
            const JsonValue* vb = nullptr;
            // Directly access object properties
            if (a.value && a.value->isObject()) {
                const auto* obj = a.value->getObject();
                auto it = obj->find(orderExpression_);
                if (it != obj->end()) va = &it->second;
            }
            if (b.value && b.value->isObject()) {
                const auto* obj = b.value->getObject();
                auto it = obj->find(orderExpression_);
                if (it != obj->end()) vb = &it->second;
            }
            // Support sorting by number and string
            if (va && vb) {
                if (va->isNumber() && vb->isNumber()) {
                    auto na = va->getNumber();
                    auto nb = vb->getNumber();
                    if (na && nb) return orderAscending_ ? (*na < *nb) : (*na > *nb);
                } else if (va->isString() && vb->isString()) {
                    auto sa = va->getString();
                    auto sb = vb->getString();
                    if (sa && sb) return orderAscending_ ? (*sa < *sb) : (*sa > *sb);
                }
            }
    // Objects without the sort field are placed at the end
            if (!va && vb) return false;
            if (va && !vb) return true;
            return false;
        });
    }
    
    // Skip the specified number of results
    if (skipCount_ > 0 && skipCount_ < results.size()) {
        results.erase(results.begin(), results.begin() + skipCount_);
    }
    
    // Limit the number of results
    if (limitCount_ > 0 && results.size() > limitCount_) {
        results.resize(limitCount_);
    }
    
    return results;
}

std::map<std::string, std::vector<JsonFilter::QueryResult>> JsonFilter::QueryBuilder::executeGrouped() const {
    std::vector<QueryResult> results = execute();
    std::map<std::string, std::vector<QueryResult>> grouped;
    if (groupByExpression_.empty()) {
        grouped["__all__"] = std::move(results);
        return grouped;
    }
    for (const auto& result : results) {
        std::string key = "__none__";
        if (result.value && result.value->isObject()) {
            // Prefer to get the group key directly from the object property
            const auto* obj = result.value->getObject();
            auto it = obj->find(groupByExpression_);
            if (it != obj->end()) {
                if (it->second.isString()) {
                    auto s = it->second.getString();
                    key = (s && !s->empty()) ? std::string(*s) : "__none__";
                } else if (it->second.isNumber()) {
                    auto n = it->second.getNumber();
                    key = n ? std::to_string(*n) : "__none__";
                }
            } else {
                // fallback: Try to use selectFirst
                auto sub = filter_.selectFirst(*result.value, groupByExpression_);
                if (sub && sub->isString()) {
                    auto s = sub->getString();
                    key = (s && !s->empty()) ? std::string(*s) : "__none__";
                } else if (sub && sub->isNumber()) {
                    auto n = sub->getNumber();
                    key = n ? std::to_string(*n) : "__none__";
                }
            }
        } else if (result.value) {
            // fallback: Try to use selectFirst
            auto sub = filter_.selectFirst(*result.value, groupByExpression_);
            if (sub && sub->isString()) {
                auto s = sub->getString();
                key = (s && !s->empty()) ? std::string(*s) : "__none__";
            } else if (sub && sub->isNumber()) {
                auto n = sub->getNumber();
                key = n ? std::to_string(*n) : "__none__";
            }
        }
        grouped[key].push_back(result);
    }
    return grouped;
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

// === Internal implementation methods ===

std::vector<JsonFilter::QueryResult> JsonFilter::executeQuery(
    const JsonValue& jsonValue, 
    const std::string& expression,
    FilterStrategy strategy) const {
    
    // JSONPath filter expression support toggle
    if (!options_.filterExpressionsEnabled && expression.find("[?(") != std::string::npos) {
        throw std::runtime_error("Filter expressions are disabled in current configuration");
    }
    
    // Check cache
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
            // Use unified JSONPath engine
            results = executeJsonPathUnified(jsonValue, expression);
            break;
        }
        default:
            // Other strategies unchanged
            break;
    }
    
    // Cache result
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

// === Cache management implementation ===

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

// === Utility method implementations ===

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
