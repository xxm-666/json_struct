
#pragma once
#include <string>
#include <vector>
#include <stack>
#include <optional>
#include <memory>
#include "json_filter.h" // Use types from json_filter.h
#include "json_path.h"

namespace JsonStruct {

class LazyQueryGenerator {
public:
    LazyQueryGenerator(const JsonFilter* filter, const JsonValue* root, const std::string& expression);
    LazyQueryGenerator(const JsonFilter* filter, const JsonValue* root, FilterFunction func);
    LazyQueryGenerator(const JsonFilter* filter, const JsonValue* root, const std::string& expression, size_t maxResults);
    bool hasNext() const;
    QueryResult next();
    std::vector<QueryResult> nextBatch(size_t maxCount = 10);
private:
    struct Frame {
        const JsonValue* value;
        std::string path;
        size_t nodeIndex = 0;
        enum class RecursiveState { None, SearchingSelf, SearchingChildren };
        RecursiveState recursiveState = RecursiveState::None;
        std::string recursiveProperty;
        std::vector<std::pair<std::string, const JsonValue*>> children;
        size_t childIndex = 0;
        size_t arrayIndex = 0;
        size_t arraySize = 0;
        Frame(const JsonValue* v, const std::string& p, size_t ni)
            : value(v), path(p), nodeIndex(ni) {}
    };
    const JsonFilter* filter_;
    const JsonValue* root_;
    std::string expression_;
    FilterFunction filterFunc_;
    bool useFilterFunc_ = false;
    bool initialized_ = false;
    std::unique_ptr<jsonpath::JsonPath> jsonPath_;
    std::vector<jsonpath::PathNode> nodes_;
    std::stack<Frame> stack_;
    std::optional<QueryResult> current_;
    size_t maxResults_ = 0;
    size_t resultCount_ = 0;
    void initialize();
    void advance();
    void expandFrameChildren(const Frame& frame);
    bool processNode(Frame& frame, const jsonpath::PathNode& node);
    bool processProperty(Frame& frame, const std::string& property);
    bool processIndex(Frame& frame, int index);
    bool processSlice(Frame& frame, int start, int end);
    bool processWildcard(Frame& frame);
    bool processRecursive(Frame& frame, const std::string& property);
};

} // namespace JsonStruct
