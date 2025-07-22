#include "json_filter.h"
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
            queryResults.emplace_back(&result.values[i].get(), result.paths[i], depth);
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
        auto results = optimizedArrayTraversal(jsonValue, expression, maxResults);
        if (!results.empty()) {
            return results;
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
            queryResults.emplace_back(&result.values[i].get(), result.paths[i], depth);
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

std::vector<QueryResult> JsonFilter::optimizedArrayTraversal(const JsonValue& jsonValue, const std::string& expression, size_t maxResults) const {
    std::vector<QueryResult> results;
    
    // Parse simple patterns like "$.store.book[*].title"
    // Split by [*] to get prefix and suffix
    auto wildcardPos = expression.find("[*]");
    if (wildcardPos == std::string::npos) {
        return results; // Not a simple wildcard pattern
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
                        return results; // Path not found
                    }
                }
            } else {
                return results; // Not an object, can't navigate
            }
            
            pos = (nextDot == std::string::npos) ? pathToParse.length() : nextDot + 1;
        }
    }
    
    // Current node should be an array
    if (!currentNode->isArray()) {
        return results;
    }
    
    const auto* arr = currentNode->getArray();
    if (!arr) {
        return results;
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

// === LazyQueryGenerator implementation ===

JsonFilter::LazyQueryGenerator::LazyQueryGenerator(const JsonFilter* filter, const JsonValue* root, const std::string& expression)
    : filter_(filter), root_(root), expression_(expression), useFilterFunc_(false), initialized_(false), 
      maxResults_(0), resultCount_(0), optimizedMode_(false) {
    // Delayed initialization, parse JSONPath on first call
}

JsonFilter::LazyQueryGenerator::LazyQueryGenerator(const JsonFilter* filter, const JsonValue* root, FilterFunction func)
    : filter_(filter), root_(root), filterFunc_(func), useFilterFunc_(true), initialized_(false), 
      maxResults_(0), resultCount_(0), optimizedMode_(false) {
    stack_.emplace_back(StackFrame{root_, "$", 0});
    advance();
}

JsonFilter::LazyQueryGenerator::LazyQueryGenerator(const JsonFilter* filter, const JsonValue* root, const std::string& expression, size_t maxResults)
    : filter_(filter), root_(root), expression_(expression), useFilterFunc_(false), initialized_(false), 
      maxResults_(maxResults), resultCount_(0), optimizedMode_(true) {
    // Delayed initialization with optimization hints
}

bool JsonFilter::LazyQueryGenerator::hasNext() const {
    // Check if we've reached the maximum results limit
    if (optimizedMode_ && maxResults_ > 0 && resultCount_ >= maxResults_) {
        return false;
    }
    
    if (!initialized_) {
        const_cast<LazyQueryGenerator*>(this)->initializeJsonPath();
    }
    return current_.has_value();
}

QueryResult JsonFilter::LazyQueryGenerator::next() {
    QueryResult result = *current_;
    
    // Increment result count for optimization tracking
    if (optimizedMode_) {
        resultCount_++;
    }
    
    advance();
    return result;
}

std::vector<QueryResult> JsonFilter::LazyQueryGenerator::nextBatch(size_t maxCount) {
    std::vector<QueryResult> results;
    results.reserve(maxCount);
    
    while (hasNext() && results.size() < maxCount) {
        results.push_back(next());
    }
    
    return results;
}

void JsonFilter::LazyQueryGenerator::initializeJsonPath() {
    if (!initialized_ && !useFilterFunc_) {
        try {
            // Smart optimization strategy based on result set size and query complexity
            if (optimizedMode_ && maxResults_ > 0) {
                // For medium result sets (51-1000), use batched fast path approach
                if (maxResults_ <= 1000) {
                    // Use fast path with chunked loading for better memory efficiency
                    auto fastResults = filter_->queryFast(*root_, expression_, maxResults_);
                    
                    // Convert fast results to lazy iterator compatible format
                    incrementalLazyState_ = std::make_unique<IncrementalLazyState>();
                    incrementalLazyState_->readyResults.reserve(fastResults.size());
                    
                    for (const auto& result : fastResults) {
                        incrementalLazyState_->readyResults.emplace_back(result.value, result.path);
                    }
                    
                    incrementalLazyState_->readyResultIndex = 0;
                    incrementalLazyState_->finished = false;
                    incrementalLazyState_->initialized = true;
                    
                    initialized_ = true;
                    advance(); // Get the first result
                    return;
                }
                
                // For very large result sets (>1000), use true streaming lazy evaluation
                // This is where the traditional lazy approach actually makes sense
                // But we'll optimize it further with early termination
            }
            
            // Traditional lazy path for very large result sets or when optimization is disabled
            incrementalLazyState_ = std::make_unique<IncrementalLazyState>();
            incrementalLazyState_->jsonPath = std::make_unique<jsonpath::JsonPath>(expression_);
            incrementalLazyState_->nodes = incrementalLazyState_->jsonPath->getNodes();
            
            // Initialize with root element as the first candidate
            incrementalLazyState_->currentCandidates.emplace_back(root_, "$");
            incrementalLazyState_->initialized = true;
            
            initialized_ = true;
            advance(); // Get the first result
        } catch (const std::exception& e) {
            // JSONPath parsing failed, set as initialized but no results
            initialized_ = true;
            current_.reset();
        }
    }
}

void JsonFilter::LazyQueryGenerator::advance() {
    current_.reset();
    
    // Check if we've reached the maximum results limit in optimized mode
    if (optimizedMode_ && maxResults_ > 0 && resultCount_ >= maxResults_) {
        return; // No more results needed
    }
    
    if (!useFilterFunc_ && initialized_ && incrementalLazyState_) {
        // Check if we're using fast path results
        if (optimizedMode_ && !incrementalLazyState_->readyResults.empty()) {
            // Fast path mode - return pre-computed results
            if (incrementalLazyState_->readyResultIndex < incrementalLazyState_->readyResults.size()) {
                auto& pair = incrementalLazyState_->readyResults[incrementalLazyState_->readyResultIndex++];
                size_t depth = std::count(pair.second.begin(), pair.second.end(), '.') + 
                             std::count(pair.second.begin(), pair.second.end(), '[');
                current_ = QueryResult(pair.first, pair.second, depth);
                return;
            } else {
                // All fast path results consumed
                return;
            }
        }
        
        // JSONPath mode - true incremental lazy evaluation
        advanceIncrementalLazy();
        return;
    }
    
    // FilterFunction mode - traverse the JSON structure
    while (!stack_.empty()) {
        auto frame = stack_.back();
        stack_.pop_back();

        if (filter_->matchesFilter(*frame.value, frame.path, filterFunc_)) {
            current_ = QueryResult(frame.value, frame.path, frame.depth);
            break;
        }

        if (frame.value->isObject()) {
            const auto* obj = frame.value->getObject();
            if (obj) {
                for (auto it = obj->begin(); it != obj->end(); ++it) {
                    std::string newPath = filter_->buildPath(frame.path, it->first);
                    stack_.emplace_back(StackFrame{&it->second, newPath, frame.depth + 1});
                }
            }
        } else if (frame.value->isArray()) {
            const auto* arr = frame.value->getArray();
            if (arr) {
                for (size_t i = arr->size(); i-- > 0;) {
                    std::string newPath = filter_->buildArrayPath(frame.path, i);
                    stack_.emplace_back(StackFrame{&(*arr)[i], newPath, frame.depth + 1});
                }
            }
        }
    }
}

void JsonFilter::LazyQueryGenerator::advanceIncrementalLazy() {
    if (incrementalLazyState_->finished) {
        return;
    }

    // First, check if we have ready results to return
    if (incrementalLazyState_->readyResultIndex < incrementalLazyState_->readyResults.size()) {
        auto& pair = incrementalLazyState_->readyResults[incrementalLazyState_->readyResultIndex++];
        size_t depth = std::count(pair.second.begin(), pair.second.end(), '.') + 
                     std::count(pair.second.begin(), pair.second.end(), '[');
        current_ = QueryResult(pair.first, pair.second, depth);
        return;
    }

    // If we've consumed all ready results and there are no more to compute, we're done
    if (incrementalLazyState_->currentNodeIndex >= incrementalLazyState_->nodes.size()) {
        incrementalLazyState_->finished = true;
        return;
    }

    // Process one batch of nodes to generate more results
    while (incrementalLazyState_->currentNodeIndex < incrementalLazyState_->nodes.size()) {
        const auto& currentNode = incrementalLazyState_->nodes[incrementalLazyState_->currentNodeIndex];
        incrementalLazyState_->nextCandidates.clear();

        // Process each current candidate through the current node
        for (const auto& [value, path] : incrementalLazyState_->currentCandidates) {
            std::vector<std::pair<const JsonValue*, std::string>> nodeOutputs;
            
            // Special handling for ROOT node
            if (currentNode.type == jsonpath::NodeType::ROOT) {
                // ROOT node just passes through the input
                nodeOutputs.emplace_back(value, path);
            } else {
                // Use evaluateSingleNode for other nodes
                incrementalLazyState_->jsonPath->evaluateSingleNode(currentNode, value, path, nodeOutputs);
            }
            
            // Add all outputs to next candidates
            for (const auto& output : nodeOutputs) {
                incrementalLazyState_->nextCandidates.emplace_back(output);
            }
        }

        // Move to next node
        incrementalLazyState_->currentNodeIndex++;
        
        // If this was the last node, the next candidates are our final results
        if (incrementalLazyState_->currentNodeIndex >= incrementalLazyState_->nodes.size()) {
            // All next candidates become ready results
            incrementalLazyState_->readyResults = std::move(incrementalLazyState_->nextCandidates);
            incrementalLazyState_->readyResultIndex = 0;
            break;
        } else {
            // Continue processing with next candidates as current candidates
            incrementalLazyState_->currentCandidates = std::move(incrementalLazyState_->nextCandidates);
            
            // If no candidates left, we're done
            if (incrementalLazyState_->currentCandidates.empty()) {
                incrementalLazyState_->finished = true;
                return;
            }
        }
    }

    // If we have ready results, return the first one
    if (incrementalLazyState_->readyResultIndex < incrementalLazyState_->readyResults.size()) {
        auto& pair = incrementalLazyState_->readyResults[incrementalLazyState_->readyResultIndex++];
        size_t depth = std::count(pair.second.begin(), pair.second.end(), '.') + 
                     std::count(pair.second.begin(), pair.second.end(), '[');
        current_ = QueryResult(pair.first, pair.second, depth);
    } else {
        incrementalLazyState_->finished = true;
    }
}

// === Lazy query generator methods ===

JsonFilter::LazyQueryGenerator JsonFilter::queryGenerator(const JsonValue& jsonValue, const FilterFunction& filter) const {
    return LazyQueryGenerator(this, &jsonValue, filter);
}

JsonFilter::LazyQueryGenerator JsonFilter::queryGenerator(const JsonValue& jsonValue, const std::string& expression, size_t maxResults) const {
    return LazyQueryGenerator(this, &jsonValue, expression, maxResults);
}

} // namespace JsonStruct
