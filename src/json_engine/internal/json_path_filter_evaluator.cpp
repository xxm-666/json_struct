#include "json_path_filter_evaluator.h"
#include "json_value.h"
#include <regex>
#include <algorithm>
#include <sstream>
#include <iostream>

namespace jsonpath
{
// Helper: find top-level operator (ignoring parentheses/brackets)
inline int findTopLevelOperator(const std::string &s, const std::string &op)
{
    int paren_depth = 0, bracket_depth = 0;
    for (size_t i = 0; i + op.size() <= s.size(); ++i)
    {
        char c = s[i];
        if (c == '(')
            paren_depth++;
        else if (c == ')')
            paren_depth--;
        else if (c == '[')
            bracket_depth++;
        else if (c == ']')
            bracket_depth--;
        if (paren_depth == 0 && bracket_depth == 0 && s.substr(i, op.size()) == op)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// Helper: trim leading and trailing whitespace
inline std::string trim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}
// compatible with C++17 and later
inline bool startsWith(const std::string &str, const std::string &prefix)
{
    return str.size() >= prefix.size() && str.substr(0, prefix.size()) == prefix;
}

inline bool endsWith(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix;
}

// Global method registry
std::unordered_map<std::string, MethodHandler>& FilterEvaluator::getMethodRegistry() {
    static std::unordered_map<std::string, MethodHandler> registry;
    static bool initialized = false;

    if (!initialized) {
        registry["length"] = lengthMethodHandler;
        registry["max"] = maxMethodHandler;
        registry["min"] = minMethodHandler;
        registry["size"] = sizeMethodHandler;
        initialized = true;
    }

    return registry;
}

void FilterEvaluator::initializeBuiltinMethods() {
    auto& registry = getMethodRegistry();

    // Register built-in methods
    registry["length"] = lengthMethodHandler;
    registry["max"] = maxMethodHandler;
    registry["min"] = minMethodHandler;
    registry["size"] = sizeMethodHandler;
}

void FilterEvaluator::registerMethod(const std::string& method_name, MethodHandler handler) {
    auto& registry = getMethodRegistry();
    registry[method_name] = std::move(handler);
}

void FilterEvaluator::unregisterMethod(const std::string& method_name) {
    auto& registry = getMethodRegistry();
    registry.erase(method_name);
}

void FilterEvaluator::clearMethods() {
    auto& registry = getMethodRegistry();
    registry.clear();
}

std::pair<std::string, std::string> FilterEvaluator::parseMethodCall(const std::string& expr) {
    // Look for pattern: property.method() or @.property.method()
    // Returns pair of (property_path, method_name)

    // Find the last occurrence of .method() pattern
    std::regex method_pattern(R"((.+)\.(\w+)\(\))");
    std::smatch match;

    if (std::regex_search(expr, match, method_pattern)) {
        std::string property_path = match[1].str();
        std::string method_name = match[2].str();

        // Trim whitespace
        property_path = trim(property_path);
        method_name = trim(method_name);

        return {property_path, method_name};
    }

    // Fallback: look for method() without property (implies @)
    std::regex simple_method_pattern(R"((\w+)\(\))");
    if (std::regex_search(expr, match, simple_method_pattern)) {
        std::string method_name = match[1].str();
        return {"@", trim(method_name)};
    }

    return {"", ""};
}

MethodCallResult FilterEvaluator::executeMethodCall(const std::string& expr, const JsonStruct::JsonValue& context) {
    auto [property_path, method_name] = parseMethodCall(expr);

    if (property_path.empty() || method_name.empty()) {
        return MethodCallResult(); // Failed to parse
    }

    // Get the property value
    const JsonStruct::JsonValue* prop_value = getPropertyValue(property_path, context);
    if (!prop_value) {
        return MethodCallResult(); // Property not found
    }

    // Find and execute the method handler
    auto& registry = getMethodRegistry();
    auto it = registry.find(method_name);
    if (it == registry.end()) {
        return MethodCallResult(); // Method not found
    }

    auto result = it->second(*prop_value);
    if (result.has_value()) {
        return MethodCallResult(result.value());
    }

    return MethodCallResult(); // Method execution failed
}

// Built-in method handlers
std::optional<JsonStruct::JsonValue> FilterEvaluator::lengthMethodHandler(const JsonStruct::JsonValue& value) {
    if (value.isArray()) {
        if (const auto* arr = value.getArray()) {
            return JsonStruct::JsonValue(static_cast<double>(arr->size()));
        }
    } else if (value.isString()) {
        auto str_opt = value.getString();
        if (str_opt) {
            return JsonStruct::JsonValue(static_cast<double>(str_opt.value().size()));
        }
    }
    return std::nullopt;
}

std::optional<JsonStruct::JsonValue> FilterEvaluator::maxMethodHandler(const JsonStruct::JsonValue& value) {
    if (!value.isArray()) {
        return std::nullopt;
    }

    const auto* arr = value.getArray();
    if (!arr || arr->empty()) {
        return std::nullopt;
    }

    double max_value = std::numeric_limits<double>::lowest();
    bool found_numeric = false;

    for (size_t i = 0; i < arr->size(); ++i) {
        const auto& element = (*arr)[i];
        if (element.isNumber()) {
            auto num_opt = element.getNumber();
            if (num_opt) {
                max_value = std::max(max_value, num_opt.value());
                found_numeric = true;
            }
        }
    }

    return found_numeric ? std::optional<JsonStruct::JsonValue>(max_value) : std::nullopt;
}

std::optional<JsonStruct::JsonValue> FilterEvaluator::minMethodHandler(const JsonStruct::JsonValue& value) {
    if (!value.isArray()) {
        return std::nullopt;
    }

    const auto* arr = value.getArray();
    if (!arr || arr->empty()) {
        return std::nullopt;
    }

    double min_value = std::numeric_limits<double>::max();
    bool found_numeric = false;

    for (size_t i = 0; i < arr->size(); ++i) {
        const auto& element = (*arr)[i];
        if (element.isNumber()) {
            auto num_opt = element.getNumber();
            if (num_opt) {
                min_value = std::min(min_value, num_opt.value());
                found_numeric = true;
            }
        }
    }

    return found_numeric ? std::optional<JsonStruct::JsonValue>(min_value) : std::nullopt;
}

std::optional<JsonStruct::JsonValue> FilterEvaluator::sizeMethodHandler(const JsonStruct::JsonValue& value) {
    // Alias for length
    return lengthMethodHandler(value);
}

bool FilterEvaluator::canHandle(const std::vector<PathNode> &nodes)
{
    return std::any_of(nodes.begin(), nodes.end(), [](const PathNode &node)
                        { return node.type == NodeType::FILTER && !node.filter_expr.empty(); });
}

std::pair<std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>,
            std::vector<std::string>>
FilterEvaluator::evaluate(const JsonStruct::JsonValue &root, const std::vector<PathNode> &nodes)
{
    std::vector<std::reference_wrapper<const JsonStruct::JsonValue>> results;
    std::vector<std::string> paths;

    // Simple implementation for now - just handle single filter at root level
    if (!nodes.empty() && nodes[0].type == NodeType::ROOT)
    {
        if (nodes.size() > 1 && nodes[1].type == NodeType::FILTER)
        {
            if (evaluateFilterCondition(nodes[1].filter_expr, root))
            {
                results.emplace_back(std::cref(root));
                paths.emplace_back("$");
            }
        }
    }

    return {results, paths};
}

std::pair<std::vector<std::reference_wrapper<JsonStruct::JsonValue>>,
            std::vector<std::string>>
FilterEvaluator::evaluateMutable(JsonStruct::JsonValue &root, const std::vector<PathNode> &nodes)
{
    std::vector<std::reference_wrapper<JsonStruct::JsonValue>> results;
    std::vector<std::string> paths;

    // Simple implementation for now - just handle single filter at root level
    if (!nodes.empty() && nodes[0].type == NodeType::ROOT)
    {
        if (nodes.size() > 1 && nodes[1].type == NodeType::FILTER)
        {
            if (evaluateFilterCondition(nodes[1].filter_expr, root))
            {
                results.emplace_back(std::ref(root));
                paths.emplace_back("$");
            }
        }
    }

    return {results, paths};
}

std::pair<std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>,
            std::vector<std::string>>
FilterEvaluator::evaluate(const std::string &filter_expr,
                            const std::vector<std::reference_wrapper<const JsonStruct::JsonValue>> &inputs,
                            const std::vector<std::string> &input_paths)
{
    std::vector<std::reference_wrapper<const JsonStruct::JsonValue>> results;
    std::vector<std::string> result_paths;

    for (size_t i = 0; i < inputs.size(); ++i)
    {
        if (evaluateFilterCondition(filter_expr, inputs[i].get()))
        {
            results.push_back(inputs[i]);
            result_paths.push_back(input_paths[i]);
        }
    }

    return {results, result_paths};
}

std::pair<std::vector<std::reference_wrapper<JsonStruct::JsonValue>>,
            std::vector<std::string>>
FilterEvaluator::evaluateMutable(const std::string &filter_expr,
                                    const std::vector<std::reference_wrapper<JsonStruct::JsonValue>> &inputs,
                                    const std::vector<std::string> &input_paths)
{
    std::vector<std::reference_wrapper<JsonStruct::JsonValue>> results;
    std::vector<std::string> result_paths;

    for (size_t i = 0; i < inputs.size(); ++i)
    {
        if (evaluateFilterCondition(filter_expr, inputs[i].get()))
        {
            results.push_back(inputs[i]);
            result_paths.push_back(input_paths[i]);
        }
    }

    return {results, result_paths};
}

bool FilterEvaluator::evaluateFilterCondition(const std::string &filter_expr, const JsonStruct::JsonValue &context)
{
    // Remove outer whitespace and ? markers
    std::string expr = filter_expr;
    if (!expr.empty() && expr[0] == '?')
    {
        expr = expr.substr(1);
    }

    // Remove outer parentheses if present
    if (startsWith(expr, "(") && endsWith(expr, ")"))
    {
        expr = expr.substr(1, expr.length() - 2);
    }

    expr = trim(expr);

    // OR has lower precedence, so check for top-level || first
    int or_pos = findTopLevelOperator(expr, " || ");
    if (or_pos != -1)
    {
        std::string left = trim(expr.substr(0, or_pos));
        std::string right = trim(expr.substr(or_pos + 4));
        return evaluateFilterCondition(left, context) || evaluateFilterCondition(right, context);
    }
    int and_pos = findTopLevelOperator(expr, " && ");
    if (and_pos != -1)
    {
        std::string left = trim(expr.substr(0, and_pos));
        std::string right = trim(expr.substr(and_pos + 4));
        return evaluateFilterCondition(left, context) && evaluateFilterCondition(right, context);
    }

    // Only handle nested filter if no top-level logical operator
    if (expr.find("[?") != std::string::npos)
    {
        return handleNestedFilter(expr, context);
    }

    // Handle method calls using the new registry system
    std::regex method_call_pattern(R"(\b\w+\(\))");
    if (std::regex_search(expr, method_call_pattern))
    {
        return handleMethodCalculation(expr, context);
    }

    // Handle "in" operator
    if (expr.find(" in ") != std::string::npos)
    {
        return handleInOperator(expr, context);
    }

    // Handle regex matching
    if (expr.find(" =~ ") != std::string::npos)
    {
        return handleRegexMatch(expr, context);
    }

    // Handle comparison operators
    if (expr.find(" == ") != std::string::npos)
    {
        return handleComparison(expr, "==", context);
    }
    if (expr.find(" != ") != std::string::npos)
    {
        return handleComparison(expr, "!=", context);
    }
    if (expr.find(" >= ") != std::string::npos)
    {
        return handleComparison(expr, ">=", context);
    }
    if (expr.find(" <= ") != std::string::npos)
    {
        return handleComparison(expr, "<=", context);
    }
    if (expr.find(" > ") != std::string::npos)
    {
        return handleComparison(expr, ">", context);
    }
    if (expr.find(" < ") != std::string::npos)
    {
        return handleComparison(expr, "<", context);
    }

    // Handle simple existence check
    return handleExistenceCheck(expr, context);
}

const JsonStruct::JsonValue *FilterEvaluator::getPropertyValue(const std::string &property, const JsonStruct::JsonValue &context)
{
    // Navigate to the property value
    const JsonStruct::JsonValue *prop_value = &context;

    if (startsWith(property, "@."))
    {
        std::string path = property.substr(2); // Remove "@."

        // Simple property navigation
        size_t dot_pos = 0;
        while (dot_pos != std::string::npos && prop_value != nullptr)
        {
            size_t next_dot = path.find('.', dot_pos);
            std::string segment = (next_dot == std::string::npos) ? path.substr(dot_pos) : path.substr(dot_pos, next_dot - dot_pos);

            if (prop_value->isObject() && prop_value->contains(segment))
            {
                prop_value = &(*prop_value)[segment];
                dot_pos = (next_dot == std::string::npos) ? std::string::npos : next_dot + 1;
            }
            else
            {
                return nullptr;
            }
        }
    }
    else if (property.find("@[") != std::string::npos ||
                (property.find("@ [") != std::string::npos))
    {
        // Handle bracket notation like @["key"] or @['key'] or @ [ "key" ]
        size_t bracket_start = property.find("[");
        size_t bracket_end = property.find("]");

        if (bracket_start != std::string::npos && bracket_end != std::string::npos && bracket_end > bracket_start)
        {
            std::string key = property.substr(bracket_start + 1, bracket_end - bracket_start - 1);

            // Trim whitespace
            key = trim(key);

            // Remove quotes if present
            if ((startsWith(key, "\"") && endsWith(key, "\"")) ||
                (startsWith(key, "'") && endsWith(key, "'")))
            {
                key = key.substr(1, key.length() - 2);
            }

            if (prop_value->isObject() && prop_value->contains(key))
            {
                prop_value = &(*prop_value)[key];
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }
    else if (property == "@")
    {
        // Direct context reference
        return prop_value;
    }
    else
    {
        return nullptr;
    }

    return prop_value;
}

bool FilterEvaluator::handleLogicalAnd(const std::string &expr, const JsonStruct::JsonValue &context)
{
    // Find the top-level && operator (not inside parentheses or brackets)
    int and_pos = findTopLevelOperator(expr, " && ");
    if (and_pos != -1)
    {
        std::string left = trim(expr.substr(0, and_pos));
        std::string right = trim(expr.substr(and_pos + 4));
        return evaluateFilterCondition(left, context) && evaluateFilterCondition(right, context);
    }
    return false;
}

bool FilterEvaluator::handleLogicalOr(const std::string &expr, const JsonStruct::JsonValue &context)
{
    // Find the top-level || operator (not inside parentheses or brackets)
    int or_pos = findTopLevelOperator(expr, " || ");
    if (or_pos != -1)
    {
        std::string left = trim(expr.substr(0, or_pos));
        std::string right = trim(expr.substr(or_pos + 4));
        bool left_result = evaluateFilterCondition(left, context);
        bool right_result = evaluateFilterCondition(right, context);
        return left_result || right_result;
    }
    return false;
}

bool FilterEvaluator::handleComparison(const std::string &expr, const std::string &op, const JsonStruct::JsonValue &context)
{
    size_t op_pos = expr.find(" " + op + " ");
    if (op_pos == std::string::npos)
        return false;

    std::string left = trim(expr.substr(0, op_pos));
    std::string right = trim(expr.substr(op_pos + op.length() + 2));

    // Get left side value
    const JsonStruct::JsonValue *left_value = getPropertyValue(left, context);
    if (!left_value)
        return false;

    // Handle string comparisons
    if (startsWith(right, "'") && endsWith(right, "'"))
    {
        std::string right_str = right.substr(1, right.length() - 2);
        if (left_value->isString())
        {
            auto left_str_opt = left_value->getString();
            if (!left_str_opt)
                return false;
            std::string left_str(left_str_opt.value());

            if (op == "==")
                return left_str == right_str;
            if (op == "!=")
                return left_str != right_str;
        }
        return false;
    }

    // Handle numeric comparisons
    try
    {
        double right_num = std::stod(right);

        if (left_value->isNumber())
        {
            auto left_num_opt = left_value->getNumber();
            if (!left_num_opt)
                return false;
            double left_num = left_num_opt.value();

            if (op == "==")
                return left_num == right_num;
            if (op == "!=")
                return left_num != right_num;
            if (op == ">")
                return left_num > right_num;
            if (op == "<")
                return left_num < right_num;
            if (op == ">=")
                return left_num >= right_num;
            if (op == "<=")
                return left_num <= right_num;
        }
    }
    catch (const std::exception &)
    {
        // Not a number
    }

    // Handle boolean comparisons
    if (right == "true" || right == "false")
    {
        bool right_bool = (right == "true");
        if (left_value->isBool())
        {
            auto left_bool_opt = left_value->getBool();
            if (!left_bool_opt)
                return false;
            bool left_bool = left_bool_opt.value();

            if (op == "==")
                return left_bool == right_bool;
            if (op == "!=")
                return left_bool != right_bool;
        }
    }

    // Handle null comparisons
    if (right == "null")
    {
        bool left_is_null = left_value->isNull();
        if (op == "==")
            return left_is_null;
        if (op == "!=")
            return !left_is_null;
    }

    return false;
}

bool FilterEvaluator::handleRegexMatch(const std::string &expr, const JsonStruct::JsonValue &context)
{
    size_t regex_pos = expr.find(" =~ ");
    if (regex_pos == std::string::npos)
        return false;

    std::string left = trim(expr.substr(0, regex_pos));
    std::string right = trim(expr.substr(regex_pos + 4));

    // Get left side value
    const JsonStruct::JsonValue *left_value = getPropertyValue(left, context);
    if (!left_value || !left_value->isString())
    {
        return false;
    }

    auto str_opt = left_value->getString();
    if (!str_opt)
        return false;
    std::string text(str_opt.value());

    // Remove quotes if present
    if (startsWith(right, "'") && endsWith(right, "'"))
    {
        right = right.substr(1, right.length() - 2);
    }

    // Extract regex pattern (remove / delimiters)
    if (startsWith(right, "/") && endsWith(right, "/"))
    {
        std::string pattern = right.substr(1, right.length() - 2);
        try
        {
            std::regex re(pattern);
            bool match_result = std::regex_search(text, re);
            return match_result;
        }
        catch (const std::regex_error &e)
        {
            return false;
        }
    }

    return false;
}

bool FilterEvaluator::handleInOperator(const std::string &expr, const JsonStruct::JsonValue &context)
{
    size_t in_pos = expr.find(" in ");
    if (in_pos == std::string::npos)
        return false;

    std::string left = trim(expr.substr(0, in_pos));
    std::string right = trim(expr.substr(in_pos + 4));

    // Get right side value (the array or string to search in)
    const JsonStruct::JsonValue *right_value = getPropertyValue(right, context);
    if (!right_value)
        return false;

    if (startsWith(left, "'") && endsWith(left, "'"))
    {
        left = left.substr(1, left.length() - 2);
    }

    if (const auto& arr_opt = right_value->toArray())
    {
        const auto& arr = arr_opt->get();
        for (const auto &element : arr)
        {
            auto str_opt = element.getString();
            if (str_opt && std::string(str_opt.value()) == left)
            {
                return true;
            }
        }
    }

    return false;
}

bool FilterEvaluator::handleMethodCalculation(const std::string &expr, const JsonStruct::JsonValue &context)
{
    // Use the new method registry system
    auto method_result = executeMethodCall(expr, context);
    if (!method_result.success) {
        return false; // Method call failed
    }

    // Parse the expression to find comparison operator after method call
    auto [property_path, method_name] = parseMethodCall(expr);
    if (property_path.empty() || method_name.empty()) {
        return false;
    }

    // Find what comes after the method call
    std::string method_call = property_path + "." + method_name + "()";
    size_t method_pos = expr.find(method_call);
    if (method_pos == std::string::npos) {
        // Try without property path (just method())
        method_call = method_name + "()";
        method_pos = expr.find(method_call);
        if (method_pos == std::string::npos) {
            return false;
        }
    }

    std::string rest = expr.substr(method_pos + method_call.length());

    // If there's no comparison operator, just return true if method succeeded
    if (rest.empty()) {
        return true;
    }

    // Create a temporary numeric context for comparison
    JsonStruct::JsonValue temp_context(method_result.value);
    std::string comparison_expr = "@" + rest; // Convert to @>5, @==3, etc.
    return evaluateFilterCondition(comparison_expr, temp_context);
}

bool FilterEvaluator::handleExistenceCheck(const std::string &expr, const JsonStruct::JsonValue &context)
{
    // Simple existence check - verify if a property exists
    const JsonStruct::JsonValue *prop_value = getPropertyValue(expr, context);
    return prop_value != nullptr && !prop_value->isNull();
}

bool FilterEvaluator::handleNestedFilter(const std::string &expr, const JsonStruct::JsonValue &context)
{
    // Pattern: @.property[?(...)]
    // Find the property name and the filter condition
    size_t bracket_pos = expr.find("[?");
    if (bracket_pos == std::string::npos)
    {
        return false;
    }

    std::string property_path = expr.substr(0, bracket_pos);
    std::string remaining = expr.substr(bracket_pos);

    // Trim property path
    property_path = trim(property_path);

    // Get the property value (should be an array or object)
    const JsonStruct::JsonValue *prop_value = getPropertyValue(property_path, context);
    if (!prop_value)
    {
        return false;
    }

    // Extract the filter condition from [?(...)]
    std::string filter_condition;
    int bracket_depth = 0;
    bool in_filter = false;

    for (size_t i = 0; i < remaining.length(); ++i)
    {
        char c = remaining[i];

        if (c == '[')
        {
            bracket_depth++;
            if (bracket_depth == 1 && i + 1 < remaining.length() && remaining[i + 1] == '?')
            {
                in_filter = true;
                i++; // Skip the '?'
                continue;
            }
            else if (in_filter)
            {
                // Add '[' to filter condition when inside filter
                filter_condition += c;
            }
        }
        else if (c == ']')
        {
            if (in_filter && bracket_depth > 1)
            {
                // Add ']' to filter condition when inside nested brackets
                filter_condition += c;
            }
            bracket_depth--;
            if (bracket_depth == 0 && in_filter)
            {
                break; // End of filter
            }
        }
        else if (in_filter)
        {
            // Add all characters when inside filter (remove the bracket_depth == 1 condition)
            filter_condition += c;
        }
    }

    // Trim the filter condition
    filter_condition = trim(filter_condition);

    // Apply the filter to the property value
    if (prop_value->isArray())
    {
        const auto *arr = prop_value->getArray();
        if (!arr)
            return false;

        for (size_t i = 0; i < arr->size(); ++i)
        {
            const auto &element = (*arr)[i];

            if (evaluateFilterCondition(filter_condition, element))
            {
                return true; // At least one element matches
            }
        }

        std::cout << "[FilterEvaluator] No elements matched the filter\n";
        return false;
    }
    else if (prop_value->isObject())
    {
        // For objects, test the filter against the object itself
        return evaluateFilterCondition(filter_condition, *prop_value);
    }

    std::cout << "[FilterEvaluator] Property is neither array nor object\n";
    return false;
}

} // namespace jsonpath
