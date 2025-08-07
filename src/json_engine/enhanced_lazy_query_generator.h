#pragma once

#include <string>
#include <vector>
#include <stack>
#include <optional>
#include <memory>
#include <functional>
#include <deque>
#include "json_filter.h"
#include <unordered_map>
#include <functional>
#include <chrono>

// Forward declarations to avoid circular dependency
namespace JsonStruct {
    class JsonValue;
    class JsonFilter;
    struct QueryResult;
    using FilterFunction = std::function<bool(const JsonValue& value, const std::string& path)>;
}

namespace jsonpath {
    class SimplePathEvaluator;
    class FilterEvaluator;
    class AdvancedEvaluator;
}

namespace JsonStruct {

/**
 * @brief Enhanced lazy query generator with full JSONPath support
 * 
 * This enhanced version supports all JSONPath features:
 * - Filter expressions [?(...)]
 * - Union expressions (comma-separated paths)
 * - Complex nested operations
 * - Regular expressions and method calls in filters
 * - Recursive descent with filters
 * - All node types supported by the main JSONPath engine
 */
class EnhancedLazyQueryGenerator {
public:
    /**
     * @brief Constructor for JSONPath expression with optional result limit
     */
    EnhancedLazyQueryGenerator(const JsonFilter* filter, const JsonValue* root, 
                              const std::string& expression, size_t maxResults = 0);
    
    /**
     * @brief Constructor for custom filter function
     */
    EnhancedLazyQueryGenerator(const JsonFilter* filter, const JsonValue* root, 
                              FilterFunction func);
    
    /**
     * @brief Check if more results are available
     */
    bool hasNext() const;
    
    /**
     * @brief Get the next result
     */
    QueryResult next();
    
    /**
     * @brief Get a batch of results
     */
    std::vector<QueryResult> nextBatch(size_t maxCount = 10);
    
    /**
     * @brief Reset the generator to start from beginning
     */
    void reset();
    
    /**
     * @brief Get performance statistics for debugging and optimization
     * @return Performance statistics as a formatted string
     */
    std::string getPerformanceStats() const;
    
    /**
     * @brief Enable or disable path caching for performance optimization
     */
    void enableCache(bool enable = true);
    
    /**
     * @brief Clear the path cache
     */
    void clearCache();
    
    /**
     * @brief Get current cache size
     */
    size_t getCacheSize() const;
    
    /**
     * @brief Get cache hit ratio for performance analysis
     */
    double getCacheHitRatio() const;
    
    /**
     * @brief Get the current progress information
     */
    struct Progress {
        size_t generatedCount = 0;
        size_t estimatedTotal = 0; // 0 if unknown
        bool hasEstimate = false;
    };
    
    Progress getProgress() const;

private:
    // Evaluation context for different JSONPath features
    struct EvaluationContext {
        const JsonValue* root;
        std::string expression;  // Store the original expression
        std::vector<jsonpath::PathNode> nodes;
        size_t currentNodeIndex = 0;
        bool useFilterFunc = false;
        FilterFunction filterFunc;
        size_t maxResults = 0;
        size_t resultCount = 0;
        bool isUnionExpression = false;
        std::vector<std::string> unionPaths; // For union expressions
        size_t currentUnionIndex = 0;
    };
    
    // Processing frame for traversal stack
    struct ProcessingFrame {
        const JsonValue* value;
        std::string path;
        size_t nodeIndex;
        size_t depth;
        
        // For array/object iteration
        std::vector<std::pair<std::string, const JsonValue*>> children;
        size_t childIndex = 0;
        
        // For slice operations
        int sliceStart = 0;
        int sliceEnd = 0;
        int sliceStep = 1;
        int sliceCurrentIndex = 0;
        
        // For recursive operations
        enum class RecursiveState { None, ProcessingSelf, ProcessingChildren };
        RecursiveState recursiveState = RecursiveState::None;
        std::string recursiveProperty;
        std::vector<ProcessingFrame> recursiveFrames;
        
        // For filter evaluation
        bool isFilterContext = false;
        std::string filterExpression;
        
        ProcessingFrame(const JsonValue* v, const std::string& p, size_t ni, size_t d = 0)
            : value(v), path(p), nodeIndex(ni), depth(d) {}
    };
    
    // Generator state
    const JsonFilter* filter_;
    EvaluationContext context_;
    std::stack<ProcessingFrame> processingStack_;
    std::optional<JsonStruct::QueryResult> currentResult_;
    bool initialized_ = false;
    bool exhausted_ = false;
    
    // Performance optimization state
    bool useOptimizedEvaluation_ = false;
    std::shared_ptr<jsonpath::SimplePathEvaluator> simpleEvaluator_;
    std::shared_ptr<jsonpath::FilterEvaluator> filterEvaluator_;
    std::shared_ptr<jsonpath::AdvancedEvaluator> advancedEvaluator_;
    
    // Performance metrics (for debugging and optimization)
    mutable size_t framesProcessed_ = 0;
    mutable size_t resultsGenerated_ = 0;
    
    // Path caching for performance optimization
    mutable std::unordered_map<std::string, std::vector<JsonValue>> pathCache_;
    mutable bool cacheEnabled_ = true;
    mutable std::chrono::steady_clock::time_point lastCacheCleanup_;
    static constexpr size_t MAX_CACHE_SIZE = 100;
    static constexpr std::chrono::minutes CACHE_CLEANUP_INTERVAL{5};
    
    // Initialization and setup
    void initialize();
    void parseExpression(const std::string& expression);
    void determineEvaluationStrategy();
    void setupUnionExpression(const std::string& expression);
    
    // Core evaluation logic
    void advance();
    bool processCurrentFrame();
    bool evaluateNode(ProcessingFrame& frame, const jsonpath::PathNode& node);
    
    // Node-specific processors
    bool processRootNode(ProcessingFrame& frame);
    bool processPropertyNode(ProcessingFrame& frame, const std::string& property);
    bool processIndexNode(ProcessingFrame& frame, int index);
    bool processSliceNode(ProcessingFrame& frame, int start, int end, int step);
    bool processWildcardNode(ProcessingFrame& frame);
    bool processRecursiveNode(ProcessingFrame& frame, const std::string& property);
    bool processFilterNode(ProcessingFrame& frame, const std::string& filterExpr);
    bool processUnionNode(ProcessingFrame& frame, const jsonpath::PathNode& node);
    
    // Filter evaluation support
    bool evaluateFilter(const std::string& filterExpr, const JsonValue& context, const std::string& path);
    std::vector<std::reference_wrapper<const JsonValue>> applyFilterToArray(
        const JsonValue& arrayValue, const std::string& filterExpr, const std::string& basePath);
    
    // Union expression support
    void processNextUnionPath();
    
    // Helper methods
    void expandChildren(ProcessingFrame& frame);
    void pushChildFrames(const ProcessingFrame& parentFrame, const jsonpath::PathNode& nextNode);
    std::string buildPath(const std::string& basePath, const std::string& component);
    std::string buildArrayPath(const std::string& basePath, size_t index);
    bool shouldContinueProcessing() const;
    
    // Optimization methods
    bool canUseSimpleEvaluation() const;
    bool canUseFilterEvaluation() const;
    bool canUseAdvancedEvaluation() const;
    void optimizeForSimplePath();
    void optimizeForFilterPath();
    void optimizeForAdvancedPath();
    
    // Cache management
    std::string generateCacheKey(const std::string& path, const std::string& expression) const;
    bool getCachedResults(const std::string& cacheKey, std::vector<JsonValue>& results) const;
    void storeCachedResults(const std::string& cacheKey, const std::vector<JsonValue>& results) const;
    void cleanupCache() const;
    mutable size_t cacheHits_ = 0;
    mutable size_t cacheQueries_ = 0;
};

} // namespace JsonStruct
