#include "json_path.h"
#include "json_value.h"
#include "internal/json_path_tokenizer.h"
#include "internal/json_path_parser.h"
#include "internal/json_path_simple_evaluator.h"
#include "internal/json_path_filter_evaluator.h"
#include "internal/json_path_advanced_evaluator.h"

using JsonValue = JsonStruct::JsonValue;

namespace jsonpath {

JsonPath::JsonPath(std::string expression) : expression_(std::move(expression)) {
    parseExpression();
}

void JsonPath::parseExpression() {
    try {
        // Check for union expressions (comma-separated paths at top level)
        if (JsonPathParser::hasTopLevelComma(expression_)) {
            nodes_ = JsonPathParser::parseUnionExpression(expression_);
        } else {
            // Tokenize the expression
            auto tokens = JsonPathTokenizer::tokenize(expression_);
            // Parse tokens into nodes
            nodes_ = JsonPathParser::parse(tokens);
        }
    } catch (const JsonPathTokenizerException& e) {
        throw JsonPathException("Tokenizer error: " + std::string(e.what()), e.position());
    } catch (const JsonPathParserException& e) {
        throw JsonPathException("Parser error: " + std::string(e.what()), e.position());
    }
}

QueryResult JsonPath::evaluate(const JsonValue& root) const {
    try {
        // Determine which evaluator to use based on expression complexity
        if (SimplePathEvaluator::canHandle(nodes_)) {
            auto result = SimplePathEvaluator::evaluate(root, nodes_);
            return {result.first, result.second};
        } else if (AdvancedEvaluator::canHandle(nodes_)) {
            auto result = AdvancedEvaluator::evaluate(root, nodes_);
            return {result.first, result.second};
        } else if (FilterEvaluator::canHandle(nodes_)) {
            auto result = FilterEvaluator::evaluate(root, nodes_);
            return {result.first, result.second};
        } else {
            // Mixed complexity - use advanced evaluator as fallback
            auto result = AdvancedEvaluator::evaluate(root, nodes_);
            return {result.first, result.second};
        }
    } catch (const std::exception&) {
        // Return empty result on evaluation error
        return {{}, {}};
    }
}

MutableQueryResult JsonPath::evaluateMutable(JsonValue& root) const {
    try {
        // Determine which evaluator to use based on expression complexity
        if (SimplePathEvaluator::canHandle(nodes_)) {
            auto result = SimplePathEvaluator::evaluateMutable(root, nodes_);
            return {result.first, result.second};
        } else if (FilterEvaluator::canHandle(nodes_)) {
            auto result = FilterEvaluator::evaluateMutable(root, nodes_);
            return {result.first, result.second};
        } else if (AdvancedEvaluator::canHandle(nodes_)) {
            auto result = AdvancedEvaluator::evaluateMutable(root, nodes_);
            return {result.first, result.second};
        } else {
            // Mixed complexity - use advanced evaluator as fallback
            auto result = AdvancedEvaluator::evaluateMutable(root, nodes_);
            return {result.first, result.second};
        }
    } catch (const std::exception&) {
        // Return empty result on evaluation error
        return {{}, {}};
    }
}

bool JsonPath::exists(const JsonValue& root) const {
    auto result = evaluate(root);
    return !result.empty();
}

std::optional<std::reference_wrapper<const JsonValue>>
JsonPath::selectFirst(const JsonValue& root) const {
    auto result = evaluate(root);
    return result.first();
}

std::vector<std::reference_wrapper<const JsonValue>>
JsonPath::selectAll(const JsonValue& root) const {
    auto result = evaluate(root);
    return result.values;
}

std::optional<std::reference_wrapper<JsonValue>>
JsonPath::selectFirstMutable(JsonValue& root) const {
    auto result = evaluateMutable(root);
    return result.first();
}

std::vector<std::reference_wrapper<JsonValue>>
JsonPath::selectAllMutable(JsonValue& root) const {
    auto result = evaluateMutable(root);
    return result.values;
}

bool JsonPath::isValidExpression(const std::string& expression) {
    try {
        JsonPath path(expression);
        return true;
    } catch (const JsonPathException&) {
        return false;
    }
}

JsonPath JsonPath::parse(const std::string& expression) {
    return JsonPath(expression);
}

} // namespace jsonpath

namespace jsonvalue_jsonpath {

jsonpath::QueryResult query(const JsonValue& root, const std::string& path_expression) {
    try {
        jsonpath::JsonPath path(path_expression);
        return path.evaluate(root);
    } catch (const jsonpath::JsonPathException&) {
        return {{}, {}};
    }
}

bool exists(const JsonValue& root, const std::string& path_expression) {
    try {
        jsonpath::JsonPath path(path_expression);
        return path.exists(root);
    } catch (const jsonpath::JsonPathException&) {
        return false;
    }
}

std::optional<std::reference_wrapper<const JsonValue>>
selectFirst(const JsonValue& root, const std::string& path_expression) {
    try {
        jsonpath::JsonPath path(path_expression);
        return path.selectFirst(root);
    } catch (const jsonpath::JsonPathException&) {
        return std::nullopt;
    }
}

std::vector<std::reference_wrapper<const JsonValue>>
selectAll(const JsonValue& root, const std::string& path_expression) {
    try {
        jsonpath::JsonPath path(path_expression);
        return path.selectAll(root);
    } catch (const jsonpath::JsonPathException&) {
        return {};
    }
}

jsonpath::MutableQueryResult queryMutable(JsonValue& root, const std::string& path_expression) {
    try {
        jsonpath::JsonPath path(path_expression);
        return path.evaluateMutable(root);
    } catch (const jsonpath::JsonPathException&) {
        return {{}, {}};
    }
}

std::optional<std::reference_wrapper<JsonValue>>
selectFirstMutable(JsonValue& root, const std::string& path_expression) {
    try {
        jsonpath::JsonPath path(path_expression);
        return path.selectFirstMutable(root);
    } catch (const jsonpath::JsonPathException&) {
        return std::nullopt;
    }
}

std::vector<std::reference_wrapper<JsonValue>>
selectAllMutable(JsonValue& root, const std::string& path_expression) {
    try {
        jsonpath::JsonPath path(path_expression);
        return path.selectAllMutable(root);
    } catch (const jsonpath::JsonPathException&) {
        return {};
    }
}

} // namespace jsonvalue_jsonpath
