#include "json_query_generator.h"
#include "json_value.h"
#include "lazy_query_generator.h"

namespace JsonStruct {

// === JsonQueryGenerator Implementation ===

JsonQueryGenerator::JsonQueryGenerator(const JsonValue& root, const std::string& expression,
                                       const GeneratorOptions& options)
    : root_(&root), expression_(expression), options_(options), resultsLoaded_(false) {
    reset();
}

void JsonQueryGenerator::reset() {
    state_ = State::Ready;
    currentIndex_ = 0;
    totalGenerated_ = 0;
    resultsLoaded_ = false;
    cachedResults_.clear();
    
    try {
        // 创建真正的懒加载生成器
        static JsonFilter defaultFilter = JsonFilter::createDefault();
        lazyGen_ = std::make_unique<LazyQueryGenerator>(
            defaultFilter.queryGenerator(*root_, expression_, options_.maxResults)
        );
    } catch (const std::exception&) {
        // If lazy generator creation fails (e.g., invalid expression), disable the generator
        lazyGen_ = nullptr;
    }
}

void JsonQueryGenerator::terminate() {
    state_ = State::Terminated;
}

bool JsonQueryGenerator::hasMore() const {
    if (state_ == State::Completed || state_ == State::Terminated) {
        return false;
    }
    if (options_.maxResults > 0 && totalGenerated_ >= options_.maxResults) {
        return false;
    }
    
    // 使用懒加载生成器检查是否还有更多结果
    return lazyGen_ && lazyGen_->hasNext();
}

std::optional<std::pair<const JsonValue*, std::string>> JsonQueryGenerator::getNext() {
    if (!hasMore()) {
        state_ = State::Completed;
        return std::nullopt;
    }
    if (state_ == State::Ready) {
        state_ = State::Running;
    }
    
    // 从懒加载生成器获取下一个结果
    auto queryResult = lazyGen_->next();
    ++totalGenerated_;
    
    if (options_.stopOnFirstMatch && totalGenerated_ >= 1) {
        state_ = State::Completed;
    }
    
    return std::make_pair(queryResult.value, queryResult.path);
}

JsonQueryGenerator::Iterator JsonQueryGenerator::begin() {
    reset();
    return Iterator(this, 0);
}

JsonQueryGenerator::Iterator JsonQueryGenerator::end() {
    return Iterator(nullptr, SIZE_MAX);
}

void JsonQueryGenerator::yield(const ResultYielder& yielder) {
    size_t index = 0;
    for (auto it = begin(); it != end(); ++it) {
        if (!yielder(it->first, it->second, index++)) {
            it.terminate();
            break;
        }
    }
}

std::vector<std::pair<const JsonValue*, std::string>> JsonQueryGenerator::takeBatch(size_t batchSize) {
    if (batchSize == 0) {
        batchSize = options_.batchSize;
    }
    std::vector<std::pair<const JsonValue*, std::string>> batch;
    batch.reserve(batchSize);
    for (size_t i = 0; i < batchSize && hasMore(); ++i) {
        auto next = getNext();
        if (next) {
            batch.emplace_back(*next);
        } else {
            break;
        }
    }
    return batch;
}

// === JsonQueryGenerator::Iterator Implementation ===

JsonQueryGenerator::Iterator::Iterator(JsonQueryGenerator* gen, size_t idx)
    : generator_(gen), index_(idx) {
    if (generator_ && index_ != SIZE_MAX) {
        ++(*this); // Advance to first element
    }
}

JsonQueryGenerator::Iterator::reference JsonQueryGenerator::Iterator::operator*() const {
    if (!current_.has_value()) {
        throw std::runtime_error("Iterator is not dereferenceable");
    }
    return *current_;
}

JsonQueryGenerator::Iterator::pointer JsonQueryGenerator::Iterator::operator->() const {
    if (!current_.has_value()) {
        throw std::runtime_error("Iterator is not dereferenceable");
    }
    return &(*current_);
}

JsonQueryGenerator::Iterator& JsonQueryGenerator::Iterator::operator++() {
    if (!generator_) {
        return *this;
    }
    current_ = generator_->getNext();
    if (!current_.has_value()) {
        generator_ = nullptr;
        index_ = SIZE_MAX;
    } else {
        ++index_;
    }
    return *this;
}

JsonQueryGenerator::Iterator JsonQueryGenerator::Iterator::operator++(int) {
    Iterator temp = *this;
    ++(*this);
    return temp;
}

bool JsonQueryGenerator::Iterator::operator==(const Iterator& other) const noexcept {
    return generator_ == other.generator_ && index_ == other.index_;
}

bool JsonQueryGenerator::Iterator::operator!=(const Iterator& other) const noexcept {
    return !(*this == other);
}

} // namespace JsonStruct
