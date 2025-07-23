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
      maxResults_(0), resultCount_(0) {
    // Delayed initialization
}

JsonFilter::LazyQueryGenerator::LazyQueryGenerator(const JsonFilter* filter, const JsonValue* root, FilterFunction func)
    : filter_(filter), root_(root), filterFunc_(func), useFilterFunc_(true), initialized_(false), 
      maxResults_(0), resultCount_(0) {
    // Filter function mode - not implemented in this refactor, kept for compatibility
    initialized_ = true;
}

JsonFilter::LazyQueryGenerator::LazyQueryGenerator(const JsonFilter* filter, const JsonValue* root, const std::string& expression, size_t maxResults)
    : filter_(filter), root_(root), expression_(expression), useFilterFunc_(false), initialized_(false), 
      maxResults_(maxResults), resultCount_(0) {
    // Delayed initialization with result limit
}

bool JsonFilter::LazyQueryGenerator::hasNext() const {
    // Check result limit
    if (maxResults_ > 0 && resultCount_ >= maxResults_) {
        return false;
    }
    
    if (!initialized_) {
        const_cast<LazyQueryGenerator*>(this)->initialize();
    }
    return current_.has_value();
}

QueryResult JsonFilter::LazyQueryGenerator::next() {
    if (!hasNext()) {
        throw std::runtime_error("No more results available");
    }
    
    QueryResult result = *current_;
    resultCount_++;
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

void JsonFilter::LazyQueryGenerator::initialize() {
    if (initialized_ || useFilterFunc_) {
        return;
    }
    
    try {
        // Parse JSONPath expression using existing json_path module
        jsonPath_ = std::make_unique<jsonpath::JsonPath>(expression_);
        nodes_ = jsonPath_->getNodes();
        
        // Initialize the evaluation stack
        while (!stack_.empty()) {
            stack_.pop();
        }
        stack_.emplace(root_, "$", 0);
        
        // Get the first result
        advance();
        
        initialized_ = true;
    } catch (const std::exception&) {
        // Handle parse errors gracefully
        initialized_ = true;
        current_.reset();
    }
}

void JsonFilter::LazyQueryGenerator::advance() {
    current_.reset();
    
    // Check result limit
    if (maxResults_ > 0 && resultCount_ >= maxResults_) {
        return;
    }

    while (!stack_.empty()) {
        Frame& frame = stack_.top();

        // If we've reached the end of the path, this is a result
        if (frame.nodeIndex == nodes_.size()) {
            size_t depth = std::count(frame.path.begin(), frame.path.end(), '.') + 
                         std::count(frame.path.begin(), frame.path.end(), '[');
            current_ = QueryResult(frame.value, frame.path, depth);
            stack_.pop();
            return;
        }

        const auto& node = nodes_[frame.nodeIndex];
        bool shouldContinue = processNode(frame, node);
        
        if (!shouldContinue) {
            stack_.pop();
        }
        
        // If we found a result, return it
        if (current_.has_value()) {
            return;
        }
    }
}

bool JsonFilter::LazyQueryGenerator::processNode(Frame& frame, const jsonpath::PathNode& node) {
    switch (node.type) {
        case jsonpath::NodeType::ROOT:
            frame.nodeIndex++;
            return true;
            
        case jsonpath::NodeType::PROPERTY:
            return processProperty(frame, node.property);
            
        case jsonpath::NodeType::INDEX:
            return processIndex(frame, node.index);
            
        case jsonpath::NodeType::SLICE:
            return processSlice(frame, node.slice_start, node.slice_end);
            
        case jsonpath::NodeType::WILDCARD:
            return processWildcard(frame);
            
        case jsonpath::NodeType::RECURSIVE:
            return processRecursive(frame, node.property);
            
        default:
            return false;
    }
}

bool JsonFilter::LazyQueryGenerator::processProperty(Frame& frame, const std::string& property) {
    if (frame.value->isObject()) {
        const auto* obj = frame.value->getObject();
        if (obj) {
            auto it = obj->find(property);
            if (it != obj->end()) {
                // Update current frame to point to the found property value
                frame.value = &it->second;
                frame.path += "." + property;
                frame.nodeIndex++;
                return true;
            }
        }
    }
    return false; // Not found, remove this frame
}

bool JsonFilter::LazyQueryGenerator::processIndex(Frame& frame, int index) {
    if (frame.value->isArray()) {
        const auto* arr = frame.value->getArray();
        if (arr && index >= 0 && static_cast<size_t>(index) < arr->size()) {
            frame.value = &(*arr)[index];
            frame.path += "[" + std::to_string(index) + "]";
            frame.nodeIndex++;
            return true;
        }
    }
    return false;
}

bool JsonFilter::LazyQueryGenerator::processSlice(Frame& frame, int start, int end) {
    if (!frame.value->isArray()) return false;
    
    const auto* arr = frame.value->getArray();
    if (!arr) return false;
    
    // Initialize array traversal state
    if (frame.arraySize == 0) {
        int actualEnd = end == -1 ? static_cast<int>(arr->size()) : end;
        if (start < 0) start = 0;
        if (actualEnd > static_cast<int>(arr->size())) actualEnd = static_cast<int>(arr->size());
        
        frame.arrayIndex = start;
        frame.arraySize = actualEnd;
    }
    
    // Check if there are more elements to process
    if (frame.arrayIndex < frame.arraySize) {
        // Create new frame for current array element
        Frame newFrame(frame.value, frame.path, frame.nodeIndex);
        newFrame.value = &(*arr)[frame.arrayIndex];
        newFrame.path = frame.path + "[" + std::to_string(frame.arrayIndex) + "]";
        newFrame.nodeIndex = frame.nodeIndex + 1;
        
        frame.arrayIndex++; // Prepare for next iteration
        
        // If this is the last element, mark current frame as complete
        if (frame.arrayIndex >= frame.arraySize) {
            stack_.pop(); // Remove current frame
        }
        
        stack_.push(newFrame);
        return true;
    }
    
    return false;
}

bool JsonFilter::LazyQueryGenerator::processWildcard(Frame& frame) {
    // Initialize iteration state on first call
    if (frame.childIndex == 0) {
        if (frame.value->isObject()) {
            const auto* obj = frame.value->getObject();
            if (!obj || obj->empty()) {
                return false;
            }
            frame.arraySize = obj->size(); // Reuse arraySize for object size
        }
        else if (frame.value->isArray()) {
            const auto* arr = frame.value->getArray();
            if (!arr || arr->empty()) {
                return false;
            }
            frame.arraySize = arr->size();
        }
        else {
            return false; // Not iterable
        }
    }
    
    // Process next child using optimized iteration
    if (frame.value->isObject()) {
        const auto* obj = frame.value->getObject();
        if (obj && frame.childIndex < frame.arraySize) {
            auto it = obj->begin();
            std::advance(it, frame.childIndex);
            
            // Pre-allocate path string for better performance
            std::string childPath;
            childPath.reserve(frame.path.size() + it->first.size() + 1);
            childPath = frame.path + "." + it->first;
            
            Frame newFrame(&it->second, std::move(childPath), frame.nodeIndex + 1);
            
            frame.childIndex++;
            
            // Remove current frame if this is the last child
            if (frame.childIndex >= frame.arraySize) {
                stack_.pop();
            }
            
            stack_.push(std::move(newFrame));
            return true;
        }
    }
    else if (frame.value->isArray()) {
        const auto* arr = frame.value->getArray();
        if (arr && frame.childIndex < frame.arraySize) {
            // Pre-allocate path string for array index
            std::string childPath;
            childPath.reserve(frame.path.size() + 12); // Estimate for "[index]"
            childPath = frame.path + "[" + std::to_string(frame.childIndex) + "]";
            
            Frame newFrame(&(*arr)[frame.childIndex], std::move(childPath), frame.nodeIndex + 1);
            
            frame.childIndex++;
            
            // Remove current frame if this is the last element
            if (frame.childIndex >= frame.arraySize) {
                stack_.pop();
            }
            
            stack_.push(std::move(newFrame));
            return true;
        }
    }
    
    return false;
}

bool JsonFilter::LazyQueryGenerator::processRecursive(Frame& frame, const std::string& property) {
    // State machine for recursive search - prevent duplicate results
    switch (frame.recursiveState) {
        case Frame::RecursiveState::None:
            frame.recursiveState = Frame::RecursiveState::SearchingSelf;
            frame.recursiveProperty = property;
            // Continue to SearchingSelf
            
        case Frame::RecursiveState::SearchingSelf: {
            bool foundMatch = false;
            // Check if current node matches
            if (!property.empty() && frame.value->isObject()) {
                const auto* obj = frame.value->getObject();
                if (obj) {
                    auto it = obj->find(property);
                    if (it != obj->end()) {
                        // Found match, create result frame
                        Frame resultFrame(&it->second, frame.path + "." + property, frame.nodeIndex + 1);
                        stack_.push(resultFrame);
                        foundMatch = true;
                    }
                }
            }
            
            // Transition to searching children state
            frame.recursiveState = Frame::RecursiveState::SearchingChildren;
            
            // Return true if we found a match, this will be processed in the next iteration
            if (foundMatch) {
                return true;
            }
            // Continue to SearchingChildren if no match found
        }
            
        case Frame::RecursiveState::SearchingChildren:
            // Initialize children list if needed
            if (frame.children.empty() && frame.childIndex == 0) {
                if (frame.value->isObject()) {
                    const auto* obj = frame.value->getObject();
                    if (obj) {
                        for (const auto& [key, child] : *obj) {
                            frame.children.emplace_back(key, &child);
                        }
                    }
                }
                else if (frame.value->isArray()) {
                    const auto* arr = frame.value->getArray();
                    if (arr) {
                        for (size_t i = 0; i < arr->size(); ++i) {
                            frame.children.emplace_back("[" + std::to_string(i) + "]", &(*arr)[i]);
                        }
                    }
                }
            }
            
            // Process next child node
            if (frame.childIndex < frame.children.size()) {
                const auto& [childPath, childValue] = frame.children[frame.childIndex];
                
                // Create recursive frame for child node
                Frame recursiveFrame(childValue, frame.path + (childPath[0] == '[' ? childPath : "." + childPath), frame.nodeIndex);
                // Important: reset recursive state for child
                recursiveFrame.recursiveState = Frame::RecursiveState::None;
                
                frame.childIndex++;
                
                // If this is the last child, remove current frame
                if (frame.childIndex >= frame.children.size()) {
                    stack_.pop();
                }
                
                stack_.push(recursiveFrame);
                return true;
            }
            
            return false; // No more children
    }
    
    return false;
}

// === Lazy query generator factory methods ===

JsonFilter::LazyQueryGenerator JsonFilter::queryGenerator(const JsonValue& jsonValue, const FilterFunction& filter) const {
    return LazyQueryGenerator(this, &jsonValue, filter);
}

JsonFilter::LazyQueryGenerator JsonFilter::queryGenerator(const JsonValue& jsonValue, const std::string& expression, size_t maxResults) const {
    return LazyQueryGenerator(this, &jsonValue, expression, maxResults);
}

} // namespace JsonStruct
