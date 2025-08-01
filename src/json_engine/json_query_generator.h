#pragma once

#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <utility>
#include <memory>

#include "lazy_query_generator.h" // Use the new LazyQueryGenerator header

namespace JsonStruct {

// Forward declaration
class JsonValue;

/**
 * @brief Independent JSON query generator for streaming and lazy evaluation
 * 
 * This class provides memory-efficient, on-demand result generation for JSON queries.
 * It implements the generator pattern to support large datasets and early termination.
 */
class JsonQueryGenerator {
public:
    struct GeneratorOptions {
        size_t maxResults = 0;              // Maximum results to generate (0 = unlimited)
        bool stopOnFirstMatch = false;      // Stop after first match
        size_t batchSize = 100;             // Batch size for streaming
        bool enableEarlyTermination = true; // Allow early termination via callback

        GeneratorOptions()
        : maxResults(0),
          stopOnFirstMatch(false),
          batchSize(100),
          enableEarlyTermination(true) {}
    };

    // Result yielder - allows custom processing during generation
    using ResultYielder = std::function<bool(const JsonValue*, const std::string& path, size_t index)>;
    
    // Generator state
    enum class State { Ready, Running, Completed, Terminated };

private:
    const JsonValue* root_;
    std::string expression_;
    GeneratorOptions options_;
    State state_ = State::Ready;
    size_t currentIndex_ = 0;
    size_t totalGenerated_ = 0;
    
    // Cached results for streaming (lazy-loaded)
    mutable std::unique_ptr<LazyQueryGenerator> lazyGen_;
    mutable std::vector<const JsonValue*> cachedResults_;
    mutable bool resultsLoaded_ = false;

public:
    explicit JsonQueryGenerator(const JsonValue& root, const std::string& expression, 
                              const GeneratorOptions& options = {});
    
    // Non-copyable but movable
    JsonQueryGenerator(const JsonQueryGenerator&) = delete;
    JsonQueryGenerator& operator=(const JsonQueryGenerator&) = delete;
    JsonQueryGenerator(JsonQueryGenerator&&) = default;
    JsonQueryGenerator& operator=(JsonQueryGenerator&&) = default;
    
    // Iterator-like interface
    class Iterator {
    private:
        JsonQueryGenerator* generator_;
        std::optional<std::pair<const JsonValue*, std::string>> current_;
        size_t index_;
        
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = std::pair<const JsonValue*, std::string>;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;
        
        explicit Iterator(JsonQueryGenerator* gen = nullptr, size_t idx = 0);
        
        reference operator*() const;
        pointer operator->() const;
        Iterator& operator++();
        Iterator operator++(int);
        bool operator==(const Iterator& other) const noexcept;
        bool operator!=(const Iterator& other) const noexcept;
        
        // Additional methods for streaming control
        size_t getIndex() const noexcept { return index_; }
        void terminate() { if (generator_) generator_->terminate(); }
    };
    
    Iterator begin();
    Iterator end();
    
    // Generator control
    void reset();
    void terminate();
    State getState() const noexcept { return state_; }
    size_t getTotalGenerated() const noexcept { return totalGenerated_; }
    
    // Check if more results are available
    bool hasMore() const;
    
    // Get next result (internal method for iterator)
    std::optional<std::pair<const JsonValue*, std::string>> getNext();
    
    // Streaming methods with custom processing
    template<typename Processor>
    void forEach(Processor&& processor) {
        static_assert(std::is_invocable_v<Processor, const JsonValue*, const std::string&, size_t>,
                     "Processor must be callable with (const JsonValue*, const std::string&, size_t)");
        
        for (auto it = begin(); it != end(); ++it) {
            if (!processor(it->first, it->second, it.getIndex())) {
                it.terminate();
                break;
            }
        }
    }
    
    // Yield results with custom handler (returns false to stop generation)
    void yield(const ResultYielder& yielder);
    
    // Collect results in batches
    std::vector<std::pair<const JsonValue*, std::string>> takeBatch(size_t batchSize = 0);
};

/**
 * @brief Factory class for creating query generators and performing streaming operations
 */
class JsonStreamingQuery {
public:
    using GeneratorOptions = JsonQueryGenerator::GeneratorOptions;
    
    /**
     * @brief Create streaming query generator
     * @param root JSON value to query
     * @param expression JSONPath expression to evaluate
     * @param options Generator configuration options
     * @return JsonQueryGenerator for lazy evaluation
     */
    static JsonQueryGenerator createGenerator(const JsonValue& root, const std::string& expression, 
                                            const GeneratorOptions& options = {}) {
        return JsonQueryGenerator(root, expression, options);
    }
    
    /**
     * @brief Lazy query that processes results on-demand
     * @param root JSON value to query
     * @param expression JSONPath expression to evaluate
     * @param processor Function to process each result (return false to stop)
     * @param options Generator configuration options
     * @return Number of results processed before termination
     */
    template<typename Processor>
    static size_t lazyQuery(const JsonValue& root, const std::string& expression, 
                           Processor&& processor, const GeneratorOptions& options = {}) {
        static_assert(std::is_invocable_v<Processor, const JsonValue*, const std::string&>,
                     "Processor must be callable with (const JsonValue*, const std::string&)");
        
        auto generator = createGenerator(root, expression, options);
        size_t processed = 0;
        
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            ++processed;
            if (!processor(it->first, it->second)) {
                break;
            }
        }
        
        return processed;
    }
    
    /**
     * @brief Find first matching result efficiently (early termination)
     * @param root JSON value to query
     * @param expression JSONPath expression to evaluate
     * @return Pair of (pointer to value, path) or nullopt if not found
     */
    static std::optional<std::pair<const JsonValue*, std::string>> findFirst(
        const JsonValue& root, const std::string& expression) {
        GeneratorOptions opts;
        opts.stopOnFirstMatch = true;
        opts.maxResults = 1;
        
        auto generator = createGenerator(root, expression, opts);
        auto it = generator.begin();
        
        if (it != generator.end()) {
            return *it;
        }
        return std::nullopt;
    }
    
    /**
     * @brief Count matching results without materializing them
     * @param root JSON value to query
     * @param expression JSONPath expression to evaluate
     * @param maxCount Maximum count to check (0 = unlimited)
     * @return Number of matching results
     */
    static size_t countMatches(const JsonValue& root, const std::string& expression, 
                              size_t maxCount = 0) {
        GeneratorOptions opts;
        opts.maxResults = maxCount;
        
        size_t count = 0;
        auto generator = createGenerator(root, expression, opts);
        
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            ++count;
        }
        
        return count;
    }
    
    /**
     * @brief Stream values to output iterator
     * @param root JSON value to query
     * @param expression JSONPath expression to evaluate
     * @param output Output iterator to write results to
     * @param options Generator configuration options
     * @return Number of results written
     */
    template<typename OutputIterator>
    static size_t streamTo(const JsonValue& root, const std::string& expression, 
                          OutputIterator output, const GeneratorOptions& options = {}) {
        auto generator = createGenerator(root, expression, options);
        size_t count = 0;
        
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            *output++ = std::make_pair(it->first, it->second);
            ++count;
        }
        
        return count;
    }
};

} // namespace JsonStruct
