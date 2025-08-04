#pragma once

#include "json_path_parser.h"
#include <functional>
#include <vector>
#include <string>

// Forward declaration
namespace JsonStruct {
    class JsonValue;
}

namespace jsonpath {

/**
 * @brief Simple path evaluator - handles basic path traversal without complex features
 * 
 * Handles:
 * - Root access ($)
 * - Property access ($.property, $['property'])
 * - Array indexing ($.array[0], $.array[-1])
 * - Array slicing ($.array[1:3], $.array[1:3:2])
 * - Wildcard access ($.*, $[*])
 * 
 * Does NOT handle:
 * - Filters [?(...)]
 * - Recursive descent (..)
 * - Union operations
 * - Complex expressions
 */
class SimplePathEvaluator {
public:
    /**
     * @brief Check if a path can be handled by simple evaluator
     * @param nodes The parsed path nodes
     * @return True if all nodes are simple
     */
    static bool canHandle(const std::vector<PathNode>& nodes);
    
    /**
     * @brief Evaluate simple path against const JSON value
     * @param root The root JSON value
     * @param nodes The parsed path nodes
     * @return Vector of const references to matching values and their paths
     */
    static std::pair<std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>, 
                     std::vector<std::string>>
    evaluate(const JsonStruct::JsonValue& root, const std::vector<PathNode>& nodes);
    
    /**
     * @brief Evaluate simple path against mutable JSON value
     * @param root The root JSON value
     * @param nodes The parsed path nodes
     * @return Vector of mutable references to matching values and their paths
     */
    static std::pair<std::vector<std::reference_wrapper<JsonStruct::JsonValue>>, 
                     std::vector<std::string>>
    evaluateMutable(JsonStruct::JsonValue& root, const std::vector<PathNode>& nodes);

    /**
     * @brief Evaluate a single node (exposed for use by other evaluators)
     */
    template<typename ValueType>
    static void evaluateNode(const PathNode& node,
                           const std::vector<std::reference_wrapper<ValueType>>& inputs,
                           const std::vector<std::string>& input_paths,
                           std::vector<std::reference_wrapper<ValueType>>& outputs,
                           std::vector<std::string>& output_paths);

private:
    
    template<typename ValueType>
    static void evaluateProperty(const std::string& property,
                               const std::vector<std::reference_wrapper<ValueType>>& inputs,
                               const std::vector<std::string>& input_paths,
                               std::vector<std::reference_wrapper<ValueType>>& outputs,
                               std::vector<std::string>& output_paths);
    
    template<typename ValueType>
    static void evaluateIndex(int index,
                            const std::vector<std::reference_wrapper<ValueType>>& inputs,
                            const std::vector<std::string>& input_paths,
                            std::vector<std::reference_wrapper<ValueType>>& outputs,
                            std::vector<std::string>& output_paths);
    
    template<typename ValueType>
    static void evaluateSlice(int start, int end, int step,
                            const std::vector<std::reference_wrapper<ValueType>>& inputs,
                            const std::vector<std::string>& input_paths,
                            std::vector<std::reference_wrapper<ValueType>>& outputs,
                            std::vector<std::string>& output_paths);
    
    template<typename ValueType>
    static void evaluateWildcard(const std::vector<std::reference_wrapper<ValueType>>& inputs,
                               const std::vector<std::string>& input_paths,
                               std::vector<std::reference_wrapper<ValueType>>& outputs,
                               std::vector<std::string>& output_paths);
    
    static int normalizeArrayIndex(int index, size_t array_size);
};

} // namespace jsonpath
