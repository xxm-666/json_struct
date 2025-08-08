#include "json_filter.h"
#include "lazy_query_generator.h"
#include <algorithm>
#include <stdexcept>
#include <memory>

namespace JsonStruct {

// === JsonFilter implementation ===

JsonFilter JsonFilter::createDefault() {
    return JsonFilter{};
}

JsonFilter JsonFilter::createWithOptions(const JsonFilterOptions& options) {
    return JsonFilter{options};
}

JsonFilter::JsonFilter(const JsonFilterOptions& options) : options_(options) {
}

std::vector<QueryResult> JsonFilter::query(const JsonValue& jsonValue, const std::string& expression) const {
    try {
        jsonpath::JsonPath jsonPath(expression);
        auto result = jsonPath.evaluate(jsonValue);
        
        std::vector<QueryResult> queryResults;
        queryResults.reserve(result.values.size());
        
        for (size_t i = 0; i < result.values.size(); ++i) {
            size_t depth = std::count(result.paths[i].begin(), result.paths[i].end(), '.') + 
                          std::count(result.paths[i].begin(), result.paths[i].end(), '[');
            // Create a copy of the JsonValue to avoid dangling pointers
            queryResults.emplace_back(new JsonValue(result.values[i].get()), result.paths[i], depth);
        }
        
        return queryResults;
    } catch (const std::exception&) {
        return {};
    }
}

std::vector<QueryResult> JsonFilter::queryFast(const JsonValue& jsonValue, const std::string& expression, size_t maxResults) const {
    // Fast path optimization for common patterns
    if (maxResults == 0) {
        return query(jsonValue, expression); // No limit, use standard method
    }
    
    // Check for simple array traversal patterns like "$.store.book[*].title"
    if (expression.find("[*]") != std::string::npos && expression.find("..") == std::string::npos) {
        // Parse the path components manually for simple patterns
        if (auto results = optimizedArrayTraversal(jsonValue, expression, maxResults)) {
            return *results;
        }
    }
    
    // Fallback to standard JSONPath evaluation with early termination
    try {
        jsonpath::JsonPath jsonPath(expression);
        auto result = jsonPath.evaluate(jsonValue);
        
        std::vector<QueryResult> queryResults;
        size_t actualLimit = std::min(maxResults, result.values.size());
        queryResults.reserve(actualLimit);
        
        for (size_t i = 0; i < actualLimit; ++i) {
            size_t depth = std::count(result.paths[i].begin(), result.paths[i].end(), '.') + 
                          std::count(result.paths[i].begin(), result.paths[i].end(), '[');
            // Create a copy of the JsonValue to avoid dangling pointers
            queryResults.emplace_back(new JsonValue(result.values[i].get()), result.paths[i], depth);
        }
        
        return queryResults;
    } catch (const std::exception&) {
        return {};
    }
}

std::vector<QueryResult> JsonFilter::query(const JsonValue& jsonValue, const FilterFunction& filter) const {
    std::vector<QueryResult> results;
    
    std::function<void(const JsonValue&, const std::string&, size_t)> traverse;
    traverse = [&](const JsonValue& current, const std::string& currentPath, size_t depth) {
        if (matchesFilter(current, currentPath, filter)) {
            results.emplace_back(&current, currentPath, depth);
        }
        
        if (current.isObject()) {
            const auto* obj = current.getObject();
            if (obj) {
                for (const auto& [key, value] : *obj) {
                    std::string newPath = buildPath(currentPath, key);
                    traverse(value, newPath, depth + 1);
                }
            }
        } else if (current.isArray()) {
            const auto* arr = current.getArray();
            if (arr) {
                for (size_t i = 0; i < arr->size(); ++i) {
                    std::string newPath = buildArrayPath(currentPath, i);
                    traverse((*arr)[i], newPath, depth + 1);
                }
            }
        }
    };
    
    traverse(jsonValue, "$", 0);
    return results;
}

std::optional<QueryResult> JsonFilter::queryFirst(const JsonValue& jsonValue, const std::string& expression) const {
    try {
        jsonpath::JsonPath jsonPath(expression);
        auto result = jsonPath.evaluate(jsonValue);
        
        if (!result.values.empty()) {
            size_t depth = std::count(result.paths[0].begin(), result.paths[0].end(), '.') + 
                          std::count(result.paths[0].begin(), result.paths[0].end(), '[');
            return QueryResult(&result.values[0].get(), result.paths[0], depth);
        }
    } catch (const std::exception&) {
        // Fall through to return nullopt
    }
    return std::nullopt;
}

bool JsonFilter::exists(const JsonValue& jsonValue, const std::string& expression) const {
    try {
        jsonpath::JsonPath jsonPath(expression);
        return jsonPath.exists(jsonValue);
    } catch (const std::exception&) {
        return false;
    }
}

size_t JsonFilter::count(const JsonValue& jsonValue, const std::string& expression) const {
    try {
        jsonpath::JsonPath jsonPath(expression);
        auto result = jsonPath.evaluate(jsonValue);
        return result.values.size();
    } catch (const std::exception&) {
        return 0;
    }
}

// Backward compatibility methods
bool JsonFilter::pathExists(const JsonValue& jsonValue, const std::string& expression) const {
    return exists(jsonValue, expression);
}

const JsonValue* JsonFilter::selectFirst(const JsonValue& jsonValue, const std::string& expression) const {
    auto result = queryFirst(jsonValue, expression);
    return result.has_value() ? result->value : nullptr;
}

std::vector<const JsonValue*> JsonFilter::selectAll(const JsonValue& jsonValue, const std::string& expression) const {
    auto results = query(jsonValue, expression);
    std::vector<const JsonValue*> values;
    values.reserve(results.size());
    
    for (const auto& result : results) {
        values.push_back(result.value);
    }
    
    return values;
}

std::vector<JsonValue> JsonFilter::selectValues(const JsonValue& jsonValue, const std::string& expression) const {
    auto results = query(jsonValue, expression);
    std::vector<JsonValue> values;
    values.reserve(results.size());
    
    for (const auto& result : results) {
        values.push_back(*result.value);
    }
    
    return values;
}

std::string JsonFilter::buildPath(const std::string& basePath, const std::string& property) const {
    if (basePath == "$") {
        return "$." + property;
    }
    return basePath + "." + property;
}

std::string JsonFilter::buildArrayPath(const std::string& basePath, size_t index) const {
    return basePath + "[" + std::to_string(index) + "]";
}

bool JsonFilter::matchesFilter(const JsonValue& value, const std::string& path, const FilterFunction& filter) const {
    return filter(value, path);
}

std::optional<std::vector<QueryResult>> JsonFilter::optimizedArrayTraversal(const JsonValue& jsonValue, const std::string& expression, size_t maxResults) const {
    std::vector<QueryResult> results;
    
    // Parse simple patterns like "$.store.book[*].title"
    // Split by [*] to get prefix and suffix
    auto wildcardPos = expression.find("[*]");
    if (wildcardPos == std::string::npos) {
        return std::nullopt; // Not a simple wildcard pattern
    }
    
    std::string prefixPath = expression.substr(0, wildcardPos);
    std::string suffixPath = expression.substr(wildcardPos + 3); // Skip "[*]"
    
    // Navigate to the array using the prefix path
    const JsonValue* currentNode = &jsonValue;
    std::string currentPath = "$";
    
    // Parse prefix path (simple dot-separated navigation)
    if (prefixPath != "$") {
        std::string pathToParse = prefixPath.substr(2); // Skip "$."
        size_t pos = 0;
        
        while (pos < pathToParse.length()) {
            size_t nextDot = pathToParse.find('.', pos);
            std::string component = (nextDot == std::string::npos) ? 
                                  pathToParse.substr(pos) : 
                                  pathToParse.substr(pos, nextDot - pos);
            
            if (currentNode->isObject()) {
                const auto* obj = currentNode->getObject();
                if (obj) {
                    auto it = obj->find(component);
                    if (it != obj->end()) {
                        currentNode = &it->second;
                        currentPath = buildPath(currentPath, component);
                    } else {
                        return std::nullopt; // Path not found
                    }
                }
            } else {
                return std::nullopt; // Not an object, can't navigate
            }
            
            pos = (nextDot == std::string::npos) ? pathToParse.length() : nextDot + 1;
        }
    }
    
    // Current node should be an array
    if (!currentNode->isArray()) {
        return std::nullopt;
    }
    
    const auto* arr = currentNode->getArray();
    if (!arr) {
        return std::nullopt;
    }
    
    // Iterate through array elements with early termination
    size_t count = 0;
    for (size_t i = 0; i < arr->size() && count < maxResults; ++i) {
        const JsonValue& element = (*arr)[i];
        std::string elementPath = buildArrayPath(currentPath, i);
        
        if (suffixPath.empty()) {
            // Return the array element itself
            results.emplace_back(&element, elementPath, 
                               std::count(elementPath.begin(), elementPath.end(), '.') + 
                               std::count(elementPath.begin(), elementPath.end(), '['));
            count++;
        } else {
            // Navigate further using suffix path
            const JsonValue* targetNode = &element;
            std::string targetPath = elementPath;
            
            // Parse suffix path (simple dot-separated navigation)
            std::string suffixToParse = suffixPath.substr(1); // Skip leading "."
            size_t pos = 0;
            bool pathValid = true;
            
            while (pos < suffixToParse.length() && pathValid) {
                size_t nextDot = suffixToParse.find('.', pos);
                std::string component = (nextDot == std::string::npos) ? 
                                      suffixToParse.substr(pos) : 
                                      suffixToParse.substr(pos, nextDot - pos);
                
                if (targetNode->isObject()) {
                    const auto* obj = targetNode->getObject();
                    if (obj) {
                        auto it = obj->find(component);
                        if (it != obj->end()) {
                            targetNode = &it->second;
                            targetPath = buildPath(targetPath, component);
                        } else {
                            pathValid = false;
                        }
                    } else {
                        pathValid = false;
                    }
                } else {
                    pathValid = false;
                }
                
                pos = (nextDot == std::string::npos) ? suffixToParse.length() : nextDot + 1;
            }
            
            if (pathValid) {
                results.emplace_back(targetNode, targetPath,
                                   std::count(targetPath.begin(), targetPath.end(), '.') + 
                                   std::count(targetPath.begin(), targetPath.end(), '['));
                count++;
            }
        }
    }
    
    return results;
}


// === Lazy query generator factory methods ===
#include "lazy_query_generator.h"

LazyQueryGenerator JsonFilter::queryGenerator(const JsonValue& jsonValue, const FilterFunction& filter) const {
    return LazyQueryGenerator(this, &jsonValue, filter);
}

LazyQueryGenerator JsonFilter::queryGenerator(const JsonValue& jsonValue, const std::string& expression, size_t maxResults) const {
    return LazyQueryGenerator(this, &jsonValue, expression, maxResults);
}

} // namespace JsonStruct
