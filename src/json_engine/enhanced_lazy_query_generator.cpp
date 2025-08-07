#include "enhanced_lazy_query_generator.h"
#include "json_value.h"
#include "internal/json_path_tokenizer.h"
#include "internal/json_path_parser.h"
#include "internal/json_path_simple_evaluator.h"
#include "internal/json_path_filter_evaluator.h"
#include "internal/json_path_advanced_evaluator.h"
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <random>
#include <iomanip>

namespace JsonStruct {

EnhancedLazyQueryGenerator::EnhancedLazyQueryGenerator(
    const JsonFilter* filter, const JsonValue* root, 
    const std::string& expression, size_t maxResults)
    : filter_(filter), exhausted_(false) {
    
    // Initialize cache
    lastCacheCleanup_ = std::chrono::steady_clock::now();
    
    context_.root = root;
    context_.expression = expression;
    context_.maxResults = maxResults;
    context_.resultCount = 0;
    context_.useFilterFunc = false;
    
    parseExpression(expression);
    determineEvaluationStrategy();
}

EnhancedLazyQueryGenerator::EnhancedLazyQueryGenerator(
    const JsonFilter* filter, const JsonValue* root, 
    FilterFunction func)
    : filter_(filter), exhausted_(false) {
    
    context_.root = root;
    context_.maxResults = 0;
    context_.resultCount = 0;
    context_.useFilterFunc = true;
    context_.filterFunc = std::move(func);
    
    // For filter functions, we use a simple traversal approach
    initialized_ = true;
    processingStack_.emplace(root, "$", 0, 0);
}

bool EnhancedLazyQueryGenerator::hasNext() const {
    if (exhausted_ || (context_.maxResults > 0 && context_.resultCount >= context_.maxResults)) {
        return false;
    }
    
    if (!initialized_) {
        const_cast<EnhancedLazyQueryGenerator*>(this)->initialize();
    }
    
    if (!currentResult_.has_value()) {
        const_cast<EnhancedLazyQueryGenerator*>(this)->advance();
    }
    
    return currentResult_.has_value();
}

JsonStruct::QueryResult EnhancedLazyQueryGenerator::next() {
    if (!hasNext()) {
        throw std::runtime_error("No more results available");
    }
    
    auto result = *currentResult_;
    currentResult_.reset();
    context_.resultCount++;
    resultsGenerated_++;  // Performance tracking
    
    return result;
}

std::vector<JsonStruct::QueryResult> EnhancedLazyQueryGenerator::nextBatch(size_t maxCount) {
    std::vector<JsonStruct::QueryResult> results;
    results.reserve(maxCount);
    
    while (hasNext() && results.size() < maxCount) {
        results.push_back(next());
    }
    
    return results;
}

void EnhancedLazyQueryGenerator::reset() {
    initialized_ = false;
    exhausted_ = false;
    context_.resultCount = 0;
    context_.currentNodeIndex = 0;
    context_.currentUnionIndex = 0;
    currentResult_.reset();
    
    // Clear stack
    while (!processingStack_.empty()) {
        processingStack_.pop();
    }
    
    // DON'T clear cache on reset - this allows cache reuse across resets
    // DON'T reset cache hit/query counters - preserve them for performance measurement
    // cacheHits_ = 0;
    // cacheQueries_ = 0;
    
    // Reset performance counters
    framesProcessed_ = 0;
    resultsGenerated_ = 0;
    
    // Force reinitialization by calling initialize again
    initialize();
}

EnhancedLazyQueryGenerator::Progress EnhancedLazyQueryGenerator::getProgress() const {
    Progress progress;
    progress.generatedCount = context_.resultCount;
    progress.estimatedTotal = 0;
    progress.hasEstimate = false;
    
    // For some simple cases, we can provide estimates
    if (!context_.useFilterFunc && !context_.isUnionExpression) {
        // This is a simplified estimation - in practice, this would need
        // much more sophisticated analysis of the JSON structure
        progress.hasEstimate = false; // Keep it simple for now
    }
    
    return progress;
}

void EnhancedLazyQueryGenerator::parseExpression(const std::string& expression) {
    try {
        // Check if this is a union expression
        if (jsonpath::JsonPathParser::hasTopLevelComma(expression)) {
            setupUnionExpression(expression);
            return;
        }
        
        // Parse the expression into nodes
        auto tokens = jsonpath::JsonPathTokenizer::tokenize(expression);
        context_.nodes = jsonpath::JsonPathParser::parse(tokens);
        
    } catch (const std::exception& e) {
        // If parsing fails, create an empty node list which will result in no matches
        context_.nodes.clear();
    }
}

void EnhancedLazyQueryGenerator::setupUnionExpression(const std::string& expression) {
    context_.isUnionExpression = true;
    
    // Parse union expression into individual paths
    size_t pos = 0;
    size_t bracketDepth = 0;
    size_t start = 0;
    
    for (size_t i = 0; i < expression.length(); ++i) {
        char c = expression[i];
        
        if (c == '[') {
            bracketDepth++;
        } else if (c == ']') {
            bracketDepth--;
        } else if (c == ',' && bracketDepth == 0) {
            // Found a top-level comma
            std::string path = expression.substr(start, i - start);
            // Trim whitespace
            size_t pathStart = path.find_first_not_of(" \t\n\r");
            size_t pathEnd = path.find_last_not_of(" \t\n\r");
            if (pathStart != std::string::npos && pathEnd != std::string::npos) {
                context_.unionPaths.push_back(path.substr(pathStart, pathEnd - pathStart + 1));
            }
            start = i + 1;
        }
    }
    
    // Add the last path
    if (start < expression.length()) {
        std::string path = expression.substr(start);
        size_t pathStart = path.find_first_not_of(" \t\n\r");
        size_t pathEnd = path.find_last_not_of(" \t\n\r");
        if (pathStart != std::string::npos && pathEnd != std::string::npos) {
            context_.unionPaths.push_back(path.substr(pathStart, pathEnd - pathStart + 1));
        }
    }
}

void EnhancedLazyQueryGenerator::determineEvaluationStrategy() {
    if (context_.useFilterFunc || context_.isUnionExpression) {
        useOptimizedEvaluation_ = false;
        return;
    }
    
    // Check if we can use specialized evaluators
    if (canUseSimpleEvaluation()) {
        optimizeForSimplePath();
    } else if (canUseFilterEvaluation()) {
        optimizeForFilterPath();
    } else if (canUseAdvancedEvaluation()) {
        optimizeForAdvancedPath();
    } else {
        useOptimizedEvaluation_ = false;
    }
}

void EnhancedLazyQueryGenerator::initialize() {
    if (initialized_) return;
    
    if (context_.useFilterFunc) {
        // Already initialized in constructor
        return;
    }
    
    if (context_.isUnionExpression) {
        processNextUnionPath();
    } else {
        // Start with root node
        processingStack_.emplace(context_.root, "$", 0, 0);
    }
    
    initialized_ = true;
}

void EnhancedLazyQueryGenerator::advance() {
    currentResult_.reset();
    
    if (exhausted_ || shouldContinueProcessing() == false) {
        return;
    }
    
    if (context_.useFilterFunc) {
        // Handle filter function case
        while (!processingStack_.empty() && !currentResult_.has_value()) {
            auto frame = processingStack_.top();
            processingStack_.pop();
            
            // Check if current value matches filter
            if (context_.filterFunc(*frame.value, frame.path)) {
                currentResult_ = JsonStruct::QueryResult(frame.value, frame.path, frame.depth);
                return;
            }
            
            // Expand children for further processing
            expandChildren(frame);
        }
        
        if (!currentResult_.has_value()) {
            exhausted_ = true;
        }
        return;
    }
    
    // Handle JSONPath expression case
    while (!processingStack_.empty() && !currentResult_.has_value()) {
        if (!processCurrentFrame()) {
            break;
        }
    }
    
    // Check if we need to process next union path
    if (!currentResult_.has_value() && context_.isUnionExpression) {
        processNextUnionPath();
        if (!processingStack_.empty()) {
            advance(); // Recursively process the next union path
        }
    }
    
    if (!currentResult_.has_value()) {
        exhausted_ = true;
    }
}

bool EnhancedLazyQueryGenerator::processCurrentFrame() {
    if (processingStack_.empty()) {
        return false;
    }

    auto frame = processingStack_.top();
    processingStack_.pop();
    
    // Performance tracking
    framesProcessed_++;
    
    // Check cache for completed results from this expression and path pattern
    if (cacheEnabled_ && frame.nodeIndex >= context_.nodes.size()) {
        // Only cache final results, not intermediate processing
        std::string cacheKey = generateCacheKey(frame.path, context_.expression);
        std::vector<JsonValue> cachedResults;
        if (getCachedResults(cacheKey, cachedResults) && !cachedResults.empty()) {
            // Use cached result
            currentResult_ = JsonStruct::QueryResult(&cachedResults[0], frame.path, frame.depth);
            resultsGenerated_++;
            return true;
        }
    }

    // Check if we've processed all nodes for this frame
    if (frame.nodeIndex >= context_.nodes.size()) {
        // We've reached the end of the path - this is a result
        currentResult_ = JsonStruct::QueryResult(frame.value, frame.path, frame.depth);
        resultsGenerated_++;
        
        // Cache this result for future queries
        if (cacheEnabled_) {
            std::string cacheKey = generateCacheKey(frame.path, context_.expression);
            std::vector<JsonValue> resultToCache = {*frame.value};
            storeCachedResults(cacheKey, resultToCache);
        }
        
        return true;
    }
    
    // Process the next node
    const auto& node = context_.nodes[frame.nodeIndex];
    return evaluateNode(frame, node);
}

bool EnhancedLazyQueryGenerator::evaluateNode(ProcessingFrame& frame, const jsonpath::PathNode& node) {
    switch (node.type) {
        case jsonpath::NodeType::ROOT:
            return processRootNode(frame);
            
        case jsonpath::NodeType::PROPERTY:
            return processPropertyNode(frame, node.property);
            
        case jsonpath::NodeType::INDEX:
            return processIndexNode(frame, node.index);
            
        case jsonpath::NodeType::SLICE:
            return processSliceNode(frame, node.slice_start, node.slice_end, node.slice_step);
            
        case jsonpath::NodeType::WILDCARD:
            return processWildcardNode(frame);
            
        case jsonpath::NodeType::RECURSIVE:
            return processRecursiveNode(frame, node.property);
            
        case jsonpath::NodeType::FILTER:
            return processFilterNode(frame, node.filter_expr);
            
        case jsonpath::NodeType::UNION:
            return processUnionNode(frame, node);
            
        default:
            return false;
    }
}

bool EnhancedLazyQueryGenerator::processRootNode(ProcessingFrame& frame) {
    // Root node just advances to the next node
    frame.nodeIndex++;
    processingStack_.push(frame);
    return true;
}

bool EnhancedLazyQueryGenerator::processPropertyNode(ProcessingFrame& frame, const std::string& property) {
    if (!frame.value->isObject()) {
        return true; // Continue processing, but this path won't yield results
    }
    
    const auto* obj = frame.value->getObject();
    if (!obj) {
        return true;
    }
    
    auto it = obj->find(property);
    if (it != obj->end()) {
        ProcessingFrame newFrame(&it->second, buildPath(frame.path, property), frame.nodeIndex + 1, frame.depth + 1);
        processingStack_.push(newFrame);
    }
    
    return true;
}

bool EnhancedLazyQueryGenerator::processIndexNode(ProcessingFrame& frame, int index) {
    if (!frame.value->isArray()) {
        return true;
    }
    
    const auto* arr = frame.value->getArray();
    if (!arr) {
        return true;
    }
    
    // Handle negative indexing
    int actualIndex = index;
    if (index < 0) {
        actualIndex = static_cast<int>(arr->size()) + index;
    }
    
    if (actualIndex >= 0 && actualIndex < static_cast<int>(arr->size())) {
        const auto& element = (*arr)[actualIndex];
        ProcessingFrame newFrame(&element, buildArrayPath(frame.path, actualIndex), frame.nodeIndex + 1, frame.depth + 1);
        processingStack_.push(newFrame);
    }
    
    return true;
}

bool EnhancedLazyQueryGenerator::processSliceNode(ProcessingFrame& frame, int start, int end, int step) {
    if (!frame.value->isArray()) {
        return true;
    }
    
    const auto* arr = frame.value->getArray();
    if (!arr || arr->empty()) {
        return true;
    }
    
    int arraySize = static_cast<int>(arr->size());
    
    // Normalize slice parameters
    int actualStart = start;
    int actualEnd = (end == INT_MAX) ? arraySize : end;
    
    if (actualStart < 0) actualStart += arraySize;
    if (actualEnd < 0) actualEnd += arraySize;
    
    actualStart = std::max(0, std::min(actualStart, arraySize));
    actualEnd = std::max(0, std::min(actualEnd, arraySize));
    
    // Generate frames for slice elements (in reverse order for correct stack processing)
    std::vector<int> indices;
    for (int i = actualStart; i < actualEnd && i < arraySize; i += step) {
        indices.push_back(i);
    }
    
    // Push in reverse order so they're processed in correct order
    for (auto it = indices.rbegin(); it != indices.rend(); ++it) {
        const auto& element = (*arr)[*it];
        ProcessingFrame newFrame(&element, buildArrayPath(frame.path, *it), frame.nodeIndex + 1, frame.depth + 1);
        processingStack_.push(newFrame);
    }
    
    return true;
}

bool EnhancedLazyQueryGenerator::processWildcardNode(ProcessingFrame& frame) {
    expandChildren(frame);
    return true;
}

bool EnhancedLazyQueryGenerator::processRecursiveNode(ProcessingFrame& frame, const std::string& property) {
    // For recursive descent, we need to:
    // 1. Check if current node matches (if property is specified)
    // 2. Recursively process all children
    
    if (frame.recursiveState == ProcessingFrame::RecursiveState::None) {
        frame.recursiveState = ProcessingFrame::RecursiveState::ProcessingSelf;
        frame.recursiveProperty = property;
    }
    
    if (frame.recursiveState == ProcessingFrame::RecursiveState::ProcessingSelf) {
        // Check if current node matches the property (if specified)
        bool matched = false;
        if (property.empty()) {
            // Empty property means match everything
            ProcessingFrame newFrame(frame.value, frame.path, frame.nodeIndex + 1, frame.depth);
            processingStack_.push(newFrame);
            matched = true;
        } else if (frame.value->isObject()) {
            const auto* obj = frame.value->getObject();
            if (obj) {
                auto it = obj->find(property);
                if (it != obj->end()) {
                    ProcessingFrame newFrame(&it->second, buildPath(frame.path, property), frame.nodeIndex + 1, frame.depth + 1);
                    processingStack_.push(newFrame);
                    matched = true;
                }
            }
        }
        
        frame.recursiveState = ProcessingFrame::RecursiveState::ProcessingChildren;
        processingStack_.push(frame);
        return true;
    }
    
    if (frame.recursiveState == ProcessingFrame::RecursiveState::ProcessingChildren) {
        // Add recursive frames for all children
        if (frame.value->isObject()) {
            const auto* obj = frame.value->getObject();
            if (obj) {
                for (const auto& [key, value] : *obj) {
                    ProcessingFrame childFrame(&value, buildPath(frame.path, key), frame.nodeIndex, frame.depth + 1);
                    childFrame.recursiveState = ProcessingFrame::RecursiveState::ProcessingSelf;
                    childFrame.recursiveProperty = property;
                    processingStack_.push(childFrame);
                }
            }
        } else if (frame.value->isArray()) {
            const auto* arr = frame.value->getArray();
            if (arr) {
                for (size_t i = 0; i < arr->size(); ++i) {
                    ProcessingFrame childFrame(&(*arr)[i], buildArrayPath(frame.path, i), frame.nodeIndex, frame.depth + 1);
                    childFrame.recursiveState = ProcessingFrame::RecursiveState::ProcessingSelf;
                    childFrame.recursiveProperty = property;
                    processingStack_.push(childFrame);
                }
            }
        }
    }
    
    return true;
}

bool EnhancedLazyQueryGenerator::processFilterNode(ProcessingFrame& frame, const std::string& filterExpr) {
    if (!frame.value->isArray()) {
        return true;
    }
    
    // Apply filter to array elements
    auto filteredElements = applyFilterToArray(*frame.value, filterExpr, frame.path);
    
    // Add matching elements to processing stack
    const auto* arr = frame.value->getArray();
    if (arr) {
        for (size_t i = 0; i < arr->size(); ++i) {
            // Check if this element passed the filter
            auto elementIt = std::find_if(filteredElements.begin(), filteredElements.end(),
                [&](const std::reference_wrapper<const JsonValue>& ref) {
                    return &ref.get() == &(*arr)[i];
                });
            
            if (elementIt != filteredElements.end()) {
                ProcessingFrame newFrame(&(*arr)[i], buildArrayPath(frame.path, i), frame.nodeIndex + 1, frame.depth + 1);
                processingStack_.push(newFrame);
            }
        }
    }
    
    return true;
}

bool EnhancedLazyQueryGenerator::processUnionNode(ProcessingFrame& frame, const jsonpath::PathNode& node) {
    // Union nodes should be handled at a higher level
    // This shouldn't be reached in normal processing
    return false;
}

void EnhancedLazyQueryGenerator::expandChildren(ProcessingFrame& frame) {
    if (frame.value->isObject()) {
        const auto* obj = frame.value->getObject();
        if (obj) {
            for (const auto& [key, value] : *obj) {
                ProcessingFrame childFrame(&value, buildPath(frame.path, key), frame.nodeIndex + 1, frame.depth + 1);
                processingStack_.push(childFrame);
            }
        }
    } else if (frame.value->isArray()) {
        const auto* arr = frame.value->getArray();
        if (arr) {
            // Push array elements in reverse order to maintain correct order when processed from stack
            for (int i = static_cast<int>(arr->size()) - 1; i >= 0; --i) {
                ProcessingFrame childFrame(&(*arr)[i], buildArrayPath(frame.path, i), frame.nodeIndex + 1, frame.depth + 1);
                processingStack_.push(childFrame);
            }
        }
    }
}

void EnhancedLazyQueryGenerator::processNextUnionPath() {
    if (!context_.isUnionExpression || context_.currentUnionIndex >= context_.unionPaths.size()) {
        return;
    }
    
    // Parse the next union path
    const std::string& path = context_.unionPaths[context_.currentUnionIndex++];
    
    try {
        auto tokens = jsonpath::JsonPathTokenizer::tokenize(path);
        context_.nodes = jsonpath::JsonPathParser::parse(tokens);
        context_.currentNodeIndex = 0;
        
        // Clear stack and start fresh with this path
        while (!processingStack_.empty()) {
            processingStack_.pop();
        }
        
        processingStack_.emplace(context_.root, "$", 0, 0);
        
    } catch (const std::exception&) {
        // If parsing fails, try the next path
        if (context_.currentUnionIndex < context_.unionPaths.size()) {
            processNextUnionPath();
        }
    }
}

bool EnhancedLazyQueryGenerator::evaluateFilter(const std::string& filterExpr, const JsonValue& context, const std::string& path) {
    try {
        return jsonpath::FilterEvaluator::evaluateFilterCondition(filterExpr, context);
    } catch (const std::exception&) {
        return false;
    }
}

std::vector<std::reference_wrapper<const JsonValue>> EnhancedLazyQueryGenerator::applyFilterToArray(
    const JsonValue& arrayValue, const std::string& filterExpr, const std::string& basePath) {
    
    std::vector<std::reference_wrapper<const JsonValue>> results;
    
    if (!arrayValue.isArray()) {
        return results;
    }
    
    const auto* arr = arrayValue.getArray();
    if (!arr) {
        return results;
    }
    
    for (size_t i = 0; i < arr->size(); ++i) {
        const auto& element = (*arr)[i];
        if (evaluateFilter(filterExpr, element, buildArrayPath(basePath, i))) {
            results.emplace_back(element);
        }
    }
    
    return results;
}

std::string EnhancedLazyQueryGenerator::buildPath(const std::string& basePath, const std::string& component) {
    if (basePath == "$") {
        return "$." + component;
    }
    return basePath + "." + component;
}

std::string EnhancedLazyQueryGenerator::buildArrayPath(const std::string& basePath, size_t index) {
    return basePath + "[" + std::to_string(index) + "]";
}

bool EnhancedLazyQueryGenerator::shouldContinueProcessing() const {
    if (context_.maxResults > 0 && context_.resultCount >= context_.maxResults) {
        return false;
    }
    return !exhausted_;
}

// Optimization methods (simplified implementations)
bool EnhancedLazyQueryGenerator::canUseSimpleEvaluation() const {
    return jsonpath::SimplePathEvaluator::canHandle(context_.nodes);
}

bool EnhancedLazyQueryGenerator::canUseFilterEvaluation() const {
    return jsonpath::FilterEvaluator::canHandle(context_.nodes);
}

bool EnhancedLazyQueryGenerator::canUseAdvancedEvaluation() const {
    return jsonpath::AdvancedEvaluator::canHandle(context_.nodes);
}

void EnhancedLazyQueryGenerator::optimizeForSimplePath() {
    useOptimizedEvaluation_ = true;
    // In a real implementation, we could use the SimplePathEvaluator
    // for better performance on simple paths
}

void EnhancedLazyQueryGenerator::optimizeForFilterPath() {
    useOptimizedEvaluation_ = true;
    // In a real implementation, we could use the FilterEvaluator
    // for better performance on filter expressions
}

void EnhancedLazyQueryGenerator::optimizeForAdvancedPath() {
    useOptimizedEvaluation_ = true;
    // In a real implementation, we could use the AdvancedEvaluator
    // for better performance on complex expressions
}

std::string EnhancedLazyQueryGenerator::getPerformanceStats() const {
    std::ostringstream oss;
    oss << "Enhanced Lazy Query Generator Performance Stats:\n";
    oss << "  Frames Processed: " << framesProcessed_ << "\n";
    oss << "  Results Generated: " << resultsGenerated_ << "\n";
    
    if (framesProcessed_ > 0) {
        double efficiency = (static_cast<double>(resultsGenerated_) / framesProcessed_) * 100.0;
        oss << "  Efficiency Ratio: " << std::fixed << std::setprecision(2) << efficiency << "%\n";
    }
    
    // Cache statistics
    if (cacheEnabled_ && cacheQueries_ > 0) {
        double hitRatio = (static_cast<double>(cacheHits_) / cacheQueries_) * 100.0;
        oss << "  Cache Hit Ratio: " << std::fixed << std::setprecision(2) << hitRatio << "%\n";
        oss << "  Cache Size: " << pathCache_.size() << " entries\n";
    }
    
    oss << "  Current State: " << (exhausted_ ? "Exhausted" : "Active");
    return oss.str();
}

void EnhancedLazyQueryGenerator::enableCache(bool enable) {
    cacheEnabled_ = enable;
    if (!enable) {
        clearCache();
    }
}

void EnhancedLazyQueryGenerator::clearCache() {
    pathCache_.clear();
    cacheHits_ = 0;
    cacheQueries_ = 0;
    lastCacheCleanup_ = std::chrono::steady_clock::now();
}

size_t EnhancedLazyQueryGenerator::getCacheSize() const {
    return pathCache_.size();
}

double EnhancedLazyQueryGenerator::getCacheHitRatio() const {
    if (cacheQueries_ == 0) return 0.0;
    return (static_cast<double>(cacheHits_) / cacheQueries_) * 100.0;
}

std::string EnhancedLazyQueryGenerator::generateCacheKey(const std::string& path, const std::string& expression) const {
    // Create a more specific cache key that includes both path pattern and expression
    // This allows caching of intermediate results that can be reused
    return expression + "#" + path;
}

bool EnhancedLazyQueryGenerator::getCachedResults(const std::string& cacheKey, std::vector<JsonValue>& results) const {
    if (!cacheEnabled_) return false;
    
    cacheQueries_++;
    auto it = pathCache_.find(cacheKey);
    if (it != pathCache_.end()) {
        results = it->second;
        cacheHits_++;
        return true;
    }
    return false;
}

void EnhancedLazyQueryGenerator::storeCachedResults(const std::string& cacheKey, const std::vector<JsonValue>& results) const {
    if (!cacheEnabled_) return;
    
    // Clean up cache periodically
    auto now = std::chrono::steady_clock::now();
    if (now - lastCacheCleanup_ > CACHE_CLEANUP_INTERVAL) {
        cleanupCache();
        lastCacheCleanup_ = now;
    }
    
    // Don't exceed maximum cache size
    if (pathCache_.size() >= MAX_CACHE_SIZE) {
        // Remove random entry to make space
        auto it = pathCache_.begin();
        std::advance(it, std::rand() % pathCache_.size());
        pathCache_.erase(it);
    }
    
    pathCache_[cacheKey] = results;
}

void EnhancedLazyQueryGenerator::cleanupCache() const {
    // Simple cleanup: remove half of the cache entries randomly
    if (pathCache_.size() > MAX_CACHE_SIZE / 2) {
        std::vector<std::string> keysToRemove;
        keysToRemove.reserve(pathCache_.size() / 2);
        
        auto it = pathCache_.begin();
        for (size_t i = 0; i < pathCache_.size() / 2 && it != pathCache_.end(); ++i, ++it) {
            keysToRemove.push_back(it->first);
        }
        
        for (const auto& key : keysToRemove) {
            pathCache_.erase(key);
        }
    }
}

} // namespace JsonStruct
