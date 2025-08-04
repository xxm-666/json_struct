#include "json_path_simple_evaluator.h"
#include "json_value.h"
#include <climits>

namespace jsonpath {

bool SimplePathEvaluator::canHandle(const std::vector<PathNode>& nodes) {
    for (const auto& node : nodes) {
        switch (node.type) {
            case NodeType::ROOT:
            case NodeType::PROPERTY:
            case NodeType::INDEX:
            case NodeType::SLICE:
            case NodeType::WILDCARD:
                break; // These are simple
            case NodeType::RECURSIVE:
            case NodeType::FILTER:
            case NodeType::UNION:
                return false; // These are complex
        }
    }
    return true;
}

std::pair<std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>, 
          std::vector<std::string>>
SimplePathEvaluator::evaluate(const JsonStruct::JsonValue& root, const std::vector<PathNode>& nodes) {
    std::vector<std::reference_wrapper<const JsonStruct::JsonValue>> current_values;
    std::vector<std::string> current_paths;
    
    // Start with root
    current_values.emplace_back(std::cref(root));
    current_paths.emplace_back("$");
    
    // Process each node
    for (size_t i = 1; i < nodes.size(); ++i) { // Skip ROOT node
        std::vector<std::reference_wrapper<const JsonStruct::JsonValue>> next_values;
        std::vector<std::string> next_paths;
        
        evaluateNode<const JsonStruct::JsonValue>(nodes[i], current_values, current_paths, next_values, next_paths);
        
        current_values = std::move(next_values);
        current_paths = std::move(next_paths);
    }
    
    return {current_values, current_paths};
}

std::pair<std::vector<std::reference_wrapper<JsonStruct::JsonValue>>, 
          std::vector<std::string>>
SimplePathEvaluator::evaluateMutable(JsonStruct::JsonValue& root, const std::vector<PathNode>& nodes) {
    std::vector<std::reference_wrapper<JsonStruct::JsonValue>> current_values;
    std::vector<std::string> current_paths;
    
    // Start with root
    current_values.emplace_back(std::ref(root));
    current_paths.emplace_back("$");
    
    // Process each node
    for (size_t i = 1; i < nodes.size(); ++i) { // Skip ROOT node
        std::vector<std::reference_wrapper<JsonStruct::JsonValue>> next_values;
        std::vector<std::string> next_paths;
        
        evaluateNode<JsonStruct::JsonValue>(nodes[i], current_values, current_paths, next_values, next_paths);
        
        current_values = std::move(next_values);
        current_paths = std::move(next_paths);
    }
    
    return {current_values, current_paths};
}

template<typename ValueType>
void SimplePathEvaluator::evaluateNode(const PathNode& node,
                                      const std::vector<std::reference_wrapper<ValueType>>& inputs,
                                      const std::vector<std::string>& input_paths,
                                      std::vector<std::reference_wrapper<ValueType>>& outputs,
                                      std::vector<std::string>& output_paths) {
    switch (node.type) {
        case NodeType::PROPERTY:
            evaluateProperty(node.property, inputs, input_paths, outputs, output_paths);
            break;
            
        case NodeType::INDEX:
            evaluateIndex(node.index, inputs, input_paths, outputs, output_paths);
            break;
            
        case NodeType::SLICE:
            evaluateSlice(node.slice_start, node.slice_end, node.slice_step, inputs, input_paths, outputs, output_paths);
            break;
            
        case NodeType::WILDCARD:
            evaluateWildcard(inputs, input_paths, outputs, output_paths);
            break;
            
        default:
            // Should not reach here if canHandle() returned true
            break;
    }
}

template<typename ValueType>
void SimplePathEvaluator::evaluateProperty(const std::string& property,
                                          const std::vector<std::reference_wrapper<ValueType>>& inputs,
                                          const std::vector<std::string>& input_paths,
                                          std::vector<std::reference_wrapper<ValueType>>& outputs,
                                          std::vector<std::string>& output_paths) {
    for (size_t i = 0; i < inputs.size(); ++i) {
        const auto& input = inputs[i].get();
        const std::string& input_path = input_paths[i];
        
        if (input.isObject() && input.contains(property)) {
            outputs.emplace_back(std::ref(const_cast<ValueType&>(input[property])));
            output_paths.emplace_back(input_path + "." + property);
        }
    }
}

template<typename ValueType>
void SimplePathEvaluator::evaluateIndex(int index,
                                       const std::vector<std::reference_wrapper<ValueType>>& inputs,
                                       const std::vector<std::string>& input_paths,
                                       std::vector<std::reference_wrapper<ValueType>>& outputs,
                                       std::vector<std::string>& output_paths) {
    for (size_t i = 0; i < inputs.size(); ++i) {
        const auto& input = inputs[i].get();
        const std::string& input_path = input_paths[i];
        
        if (input.isArray()) {
            size_t array_size = input.size();
            int normalized_index = normalizeArrayIndex(index, array_size);
            
            if (normalized_index >= 0 && static_cast<size_t>(normalized_index) < array_size) {
                outputs.emplace_back(std::ref(const_cast<ValueType&>(input[normalized_index])));
                output_paths.emplace_back(input_path + "[" + std::to_string(normalized_index) + "]");
            }
        }
    }
}

template<typename ValueType>
void SimplePathEvaluator::evaluateSlice(int start, int end, int step,
                                       const std::vector<std::reference_wrapper<ValueType>>& inputs,
                                       const std::vector<std::string>& input_paths,
                                       std::vector<std::reference_wrapper<ValueType>>& outputs,
                                       std::vector<std::string>& output_paths) {
    for (size_t i = 0; i < inputs.size(); ++i) {
        const auto& input = inputs[i].get();
        const std::string& input_path = input_paths[i];
        
        if (input.isArray()) {
            size_t array_size = input.size();
            
            int actual_start = normalizeArrayIndex(start, array_size);
            int actual_end = (end == INT_MAX) ? static_cast<int>(array_size) : normalizeArrayIndex(end, array_size);
            
            if (actual_start < 0) actual_start = 0;
            if (actual_end > static_cast<int>(array_size)) actual_end = static_cast<int>(array_size);
            
            if (step > 0) {
                // Forward slice
                for (int idx = actual_start; idx < actual_end; idx += step) {
                    if (idx >= 0 && static_cast<size_t>(idx) < array_size) {
                        outputs.emplace_back(std::ref(const_cast<ValueType&>(input[idx])));
                        output_paths.emplace_back(input_path + "[" + std::to_string(idx) + "]");
                    }
                }
            } else if (step < 0) {
                // Reverse slice  
                for (int idx = actual_start; idx > actual_end; idx += step) {
                    if (idx >= 0 && static_cast<size_t>(idx) < array_size) {
                        outputs.emplace_back(std::ref(const_cast<ValueType&>(input[idx])));
                        output_paths.emplace_back(input_path + "[" + std::to_string(idx) + "]");
                    }
                }
            }
        }
    }
}

template<typename ValueType>
void SimplePathEvaluator::evaluateWildcard(const std::vector<std::reference_wrapper<ValueType>>& inputs,
                                          const std::vector<std::string>& input_paths,
                                          std::vector<std::reference_wrapper<ValueType>>& outputs,
                                          std::vector<std::string>& output_paths) {
    for (size_t i = 0; i < inputs.size(); ++i) {
        const auto& input = inputs[i].get();
        const std::string& input_path = input_paths[i];
        
        if (input.isObject()) {
            if (const auto* obj = input.getObject()) {
                for (const auto& [key, value] : *obj) {
                    outputs.emplace_back(std::ref(const_cast<ValueType&>(value)));
                    output_paths.emplace_back(input_path + "." + key);
                }
            }
        } else if (input.isArray()) {
            if (const auto* arr = input.getArray()) {
                for (size_t idx = 0; idx < arr->size(); ++idx) {
                    outputs.emplace_back(std::ref(const_cast<ValueType&>((*arr)[idx])));
                    output_paths.emplace_back(input_path + "[" + std::to_string(idx) + "]");
                }
            }
        }
    }
}

int SimplePathEvaluator::normalizeArrayIndex(int index, size_t array_size) {
    if (index < 0) {
        return static_cast<int>(array_size) + index;
    }
    return index;
}

// Explicit template instantiations
template void SimplePathEvaluator::evaluateNode<const JsonStruct::JsonValue>(
    const PathNode& node,
    const std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& inputs,
    const std::vector<std::string>& input_paths,
    std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& outputs,
    std::vector<std::string>& output_paths);

template void SimplePathEvaluator::evaluateNode<JsonStruct::JsonValue>(
    const PathNode& node,
    const std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& inputs,
    const std::vector<std::string>& input_paths,
    std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& outputs,
    std::vector<std::string>& output_paths);

} // namespace jsonpath
