#include "lazy_query_generator.h"
#include "json_filter.h"
#include <algorithm>
#include <stdexcept>
#include <memory>

namespace JsonStruct {

LazyQueryGenerator::LazyQueryGenerator(const JsonFilter* filter, const JsonValue* root, const std::string& expression)
    : filter_(filter), root_(root), expression_(expression), useFilterFunc_(false), initialized_(false), 
      maxResults_(0), resultCount_(0) {
    // Delayed initialization
}

LazyQueryGenerator::LazyQueryGenerator(const JsonFilter* filter, const JsonValue* root, FilterFunction func)
    : filter_(filter), root_(root), filterFunc_(func), useFilterFunc_(true), initialized_(false), 
      maxResults_(0), resultCount_(0) {
    while (!stack_.empty()) {
        stack_.pop();
    }
    stack_.emplace(root_, "$", 0);
    initialized_ = true;
    advance();
}

LazyQueryGenerator::LazyQueryGenerator(const JsonFilter* filter, const JsonValue* root, const std::string& expression, size_t maxResults)
    : filter_(filter), root_(root), expression_(expression), useFilterFunc_(false), initialized_(false), 
      maxResults_(maxResults), resultCount_(0) {
    // Delayed initialization with result limit
}

bool LazyQueryGenerator::hasNext() const {
    if (maxResults_ > 0 && resultCount_ >= maxResults_) {
        return false;
    }
    if (!initialized_) {
        const_cast<LazyQueryGenerator*>(this)->initialize();
    }
    if (useFilterFunc_) {
        if (!current_.has_value()) {
            const_cast<LazyQueryGenerator*>(this)->advance();
        }
        return current_.has_value();
    }
    return current_.has_value();
}

QueryResult LazyQueryGenerator::next() {
    if (!hasNext()) {
        throw std::runtime_error("No more results available");
    }
    QueryResult result = *current_;
    resultCount_++;
    advance();
    return result;
}

std::vector<QueryResult> LazyQueryGenerator::nextBatch(size_t maxCount) {
    std::vector<QueryResult> results;
    results.reserve(maxCount);
    while (hasNext() && results.size() < maxCount) {
        results.push_back(next());
    }
    return results;
}

void LazyQueryGenerator::initialize() {
    if (initialized_) {
        return;
    }
    if (useFilterFunc_) {
        current_.reset();
        advance();
        initialized_ = true;
        return;
    }
    try {
        nodes_ = jsonpath::JsonPath::parse(expression_).getNodes();
        while (!stack_.empty()) {
            stack_.pop();
        }
        stack_.emplace(root_, "$", 0);
        advance();
        initialized_ = true;
    } catch (const std::exception&) {
        initialized_ = true;
        current_.reset();
    }
}

void LazyQueryGenerator::advance() {
    current_.reset();
    if (maxResults_ > 0 && resultCount_ >= maxResults_) {
        return;
    }
    if (useFilterFunc_) {
        while (!stack_.empty()) {
            Frame currentFrame = stack_.top();
            stack_.pop();
            if (filterFunc_(*currentFrame.value, currentFrame.path)) {
                size_t depth = std::count(currentFrame.path.begin(), currentFrame.path.end(), '.') + 
                             std::count(currentFrame.path.begin(), currentFrame.path.end(), '[');
                current_ = QueryResult(currentFrame.value, currentFrame.path, depth);
                resultCount_++;
                expandFrameChildren(currentFrame);
                return;
            }
            expandFrameChildren(currentFrame);
        }
        return;
    }
    while (!stack_.empty()) {
        Frame& frame = stack_.top();
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
        if (current_.has_value()) {
            return;
        }
    }
}

bool LazyQueryGenerator::processNode(Frame& frame, const jsonpath::PathNode& node) {
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

bool LazyQueryGenerator::processProperty(Frame& frame, const std::string& property) {
    if (frame.value->isObject()) {
        const auto* obj = frame.value->getObject();
        if (obj) {
            auto it = obj->find(property);
            if (it != obj->end()) {
                frame.value = &it->second;
                frame.path += "." + property;
                frame.nodeIndex++;
                return true;
            }
        }
    }
    return false;
}

bool LazyQueryGenerator::processIndex(Frame& frame, int index) {
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

bool LazyQueryGenerator::processSlice(Frame& frame, int start, int end) {
    if (!frame.value->isArray()) return false;
    const auto* arr = frame.value->getArray();
    if (!arr) return false;
    if (frame.arraySize == 0) {
        int actualEnd = end == -1 ? static_cast<int>(arr->size()) : end;
        if (start < 0) start = 0;
        if (actualEnd > static_cast<int>(arr->size())) actualEnd = static_cast<int>(arr->size());
        frame.arrayIndex = start;
        frame.arraySize = actualEnd;
    }
    if (frame.arrayIndex < frame.arraySize) {
        Frame newFrame(frame.value, frame.path, frame.nodeIndex);
        newFrame.value = &(*arr)[frame.arrayIndex];
        newFrame.path = frame.path + "[" + std::to_string(frame.arrayIndex) + "]";
        newFrame.nodeIndex = frame.nodeIndex + 1;
        frame.arrayIndex++;
        if (frame.arrayIndex >= frame.arraySize) {
            stack_.pop();
        }
        stack_.push(newFrame);
        return true;
    }
    return false;
}

bool LazyQueryGenerator::processWildcard(Frame& frame) {
    if (frame.childIndex == 0) {
        if (frame.value->isObject()) {
            const auto* obj = frame.value->getObject();
            if (!obj || obj->empty()) {
                return false;
            }
            frame.arraySize = obj->size();
        }
        else if (frame.value->isArray()) {
            const auto* arr = frame.value->getArray();
            if (!arr || arr->empty()) {
                return false;
            }
            frame.arraySize = arr->size();
        }
        else {
            return false;
        }
    }
    if (frame.value->isObject()) {
        const auto* obj = frame.value->getObject();
        if (obj && frame.childIndex < frame.arraySize) {
            auto it = obj->begin();
            std::advance(it, frame.childIndex);
            std::string childPath;
            childPath.reserve(frame.path.size() + it->first.size() + 1);
            childPath = frame.path + "." + it->first;
            Frame newFrame(&it->second, std::move(childPath), frame.nodeIndex + 1);
            frame.childIndex++;
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
            std::string childPath;
            childPath.reserve(frame.path.size() + 12);
            childPath = frame.path + "[" + std::to_string(frame.childIndex) + "]";
            Frame newFrame(&(*arr)[frame.childIndex], std::move(childPath), frame.nodeIndex + 1);
            frame.childIndex++;
            if (frame.childIndex >= frame.arraySize) {
                stack_.pop();
            }
            stack_.push(std::move(newFrame));
            return true;
        }
    }
    return false;
}

bool LazyQueryGenerator::processRecursive(Frame& frame, const std::string& property) {
    switch (frame.recursiveState) {
        case Frame::RecursiveState::None:
            frame.recursiveState = Frame::RecursiveState::SearchingSelf;
            frame.recursiveProperty = property;
        case Frame::RecursiveState::SearchingSelf: {
            bool foundMatch = false;
            if (!property.empty() && frame.value->isObject()) {
                const auto* obj = frame.value->getObject();
                if (obj) {
                    auto it = obj->find(property);
                    if (it != obj->end()) {
                        Frame resultFrame(&it->second, frame.path + "." + property, frame.nodeIndex + 1);
                        stack_.push(resultFrame);
                        foundMatch = true;
                    }
                }
            }
            frame.recursiveState = Frame::RecursiveState::SearchingChildren;
            if (foundMatch) {
                return true;
            }
        }
        case Frame::RecursiveState::SearchingChildren:
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
            if (frame.childIndex < frame.children.size()) {
                const auto& [childPath, childValue] = frame.children[frame.childIndex];
                Frame recursiveFrame(childValue, frame.path + (childPath[0] == '[' ? childPath : "." + childPath), frame.nodeIndex);
                recursiveFrame.recursiveState = Frame::RecursiveState::SearchingSelf;
                recursiveFrame.recursiveProperty = frame.recursiveProperty;
                stack_.push(recursiveFrame);
                frame.childIndex++;
                return true;
            } else {
                return false;
            }
    }
    return false;
}

void LazyQueryGenerator::expandFrameChildren(const Frame& frame) {
    if (frame.value->isObject()) {
        const auto* obj = frame.value->getObject();
        if (obj) {
            auto it = obj->end();
            while (it != obj->begin()) {
                --it;
                std::string childPath = frame.path + "." + it->first;
                stack_.emplace(&it->second, childPath, 0);
            }
        }
    }
    else if (frame.value->isArray()) {
        const auto* arr = frame.value->getArray();
        if (arr) {
            for (int i = static_cast<int>(arr->size()) - 1; i >= 0; --i) {
                std::string childPath = frame.path + "[" + std::to_string(i) + "]";
                stack_.emplace(&(*arr)[i], childPath, 0);
            }
        }
    }
}

} // namespace JsonStruct
