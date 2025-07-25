#pragma once
#include "json_value.h"
#include "json_filter.h"
#include <functional>
#include <vector>
#include <memory>

namespace JsonStruct {

/**
 * @brief JSON conversion and pipeline operation class
 * 
 * Provides a chainable JSON data processing pipeline, supporting:
 * - Filtering operations
 * - Transformation operations
 * - Aggregation operations
 * - Conditional branches
 */
class JsonPipeline; 

class JsonPipeline {
public:
    using TransformFunction = std::function<JsonValue(const JsonValue&)>;
    using FilterFunction = std::function<bool(const JsonValue&)>;
    using AggregateFunction = std::function<JsonValue(const std::vector<JsonValue>&)>;

    // Pipeline step base class
    class Step {
    public:
        virtual ~Step() = default;
        virtual JsonValue execute(const JsonValue& input) const = 0;
        virtual std::unique_ptr<Step> clone() const = 0;
    };

    // Transformation step
    class TransformStep : public Step {
    private:
        TransformFunction func_;
    public:
        explicit TransformStep(TransformFunction func)
            : func_(std::move(func)) {}

        JsonValue execute(const JsonValue& input) const override {
            return func_(input);
        }

        std::unique_ptr<Step> clone() const override {
            return std::make_unique<TransformStep>(func_);
        }
    };

    // Filtering step
    class FilterStep : public Step {
    private:
        FilterFunction func_;
        JsonValue defaultValue_;
    public:
        explicit FilterStep(FilterFunction func, JsonValue defaultValue = JsonValue())
            : func_(std::move(func)), defaultValue_(std::move(defaultValue)) {}

        JsonValue execute(const JsonValue& input) const override {
            if (func_(input)) {
                return input;
            }
            return defaultValue_;
        }

        std::unique_ptr<Step> clone() const override {
            return std::make_unique<FilterStep>(func_, defaultValue_);
        }
    };

    // Aggregation step
    class AggregateStep : public Step {
    private:
        AggregateFunction func_;
    public:
        explicit AggregateStep(AggregateFunction func)
            : func_(std::move(func)) {}

        JsonValue execute(const JsonValue& input) const override {
            if(const auto& array = input.toArray()) {
                return func_(*array);
            }
            return func_({input});
        }

        std::unique_ptr<Step> clone() const override {
            return std::make_unique<AggregateStep>(func_);
        }
    };

    // Branch step (moved to implementation after class definition)
    class BranchStep;

    // Constructor
    JsonPipeline() = default;
    JsonPipeline(const JsonPipeline& other) {
        for (const auto& step : other.steps_) {
            steps_.push_back(step->clone());
        }
    }
    JsonPipeline& operator=(const JsonPipeline& other) {
        if (this != &other) {
            steps_.clear();
            for (const auto& step : other.steps_) {
                steps_.push_back(step->clone());
            }
        }
        return *this;
    }

    // Add steps
    JsonPipeline& transform(TransformFunction func) {
        steps_.push_back(std::make_unique<TransformStep>(std::move(func)));
        return *this;
    }

    JsonPipeline& filter(FilterFunction func, JsonValue defaultValue = JsonValue()) {
        steps_.push_back(std::make_unique<FilterStep>(std::move(func), std::move(defaultValue)));
        return *this;
    }

    JsonPipeline& branch(FilterFunction condition, JsonPipeline thenPipe, JsonPipeline elsePipe = JsonPipeline());

    JsonPipeline& aggregate(AggregateFunction func) {
        steps_.push_back(std::make_unique<AggregateStep>(std::move(func)));
        return *this;
    }

    // Execute pipeline
    JsonValue execute(const JsonValue& input) const {
        JsonValue current = input;
        for (const auto& step : steps_) {
            current = step->execute(current);
        }
        return current;
    }

    // Merge pipelines
    JsonPipeline operator+(const JsonPipeline& other) const {
        JsonPipeline result = *this;
        for (const auto& step : other.steps_) {
            result.steps_.push_back(step->clone());
        }
        return result;
    }

    JsonPipeline then(const JsonPipeline& nextPipeline) const {
        return *this + nextPipeline;
    }

    JsonPipeline& then(TransformFunction func) {
        return transform(std::move(func));
    }
    // Apply JSONPath query transformation
    TransformFunction query(const std::string& jsonPath);

    // Helper methods
    JsonPipeline filterArray(const FilterFunction& filter) {
        return then([filter](const JsonValue& value) -> JsonValue {
            JsonValue result(JsonValue::ArrayType{});
            if (value.isArray()) {
                if(const auto& arr = value.toArray()) {
                    for (const auto& item : arr->get()) {
                        if (filter(item)) {
                            result.append(item);
                        }
                    }
                }
            } else if (filter(value)) {
                result.append(value);
            }
            return result;
        });
    }

    JsonPipeline transformArray(const TransformFunction& transform) {
        return then([transform](const JsonValue& value) -> JsonValue {
            JsonValue result(JsonValue::ArrayType{});
            if (value.isArray()) {
                if(const auto& arr = value.toArray()) {
                    for (const auto& item : arr->get()) {
                        result.append(transform(item));
                    }
                }
            } else {
                result.append(transform(value));
            }
            return result;
        });
    }

    JsonPipeline aggregateArray(const AggregateFunction& aggregate) {
        return then([aggregate](const JsonValue& value) -> JsonValue {
            if (value.isArray()) {
                if(const auto& arr = value.toArray()) {
                    return aggregate(*arr);
                }
            }
            return aggregate({value});
        });
    }

private:
    std::vector<std::unique_ptr<Step>> steps_;
};

// Branch step implementation
class JsonPipeline::BranchStep : public JsonPipeline::Step {
private:
    FilterFunction condition_;
    JsonPipeline thenPipeline_;
    JsonPipeline elsePipeline_;
public:
    BranchStep(FilterFunction condition, JsonPipeline thenPipe, JsonPipeline elsePipe)
        : condition_(std::move(condition)), thenPipeline_(std::move(thenPipe)), elsePipeline_(std::move(elsePipe)) {}

    JsonValue execute(const JsonValue& input) const override {
        if (condition_(input)) {
            return thenPipeline_.execute(input);
        } else {
            return elsePipeline_.execute(input);
        }
    }

    std::unique_ptr<Step> clone() const override {
        return std::make_unique<BranchStep>(condition_, thenPipeline_, elsePipeline_);
    }
};

/**
 * @brief Adds pipeline operation support to JsonValue
 */
inline JsonValue operator|(const JsonValue& value, const JsonPipeline& pipeline) {
    return pipeline.execute(value);
}

/**
 * @brief Common transformation functions
 */
namespace Transforms {
    // Convert value to string
    JsonValue toString(const JsonValue& value);
    
    // Convert value to number
    JsonValue toNumber(const JsonValue& value);
    
    // Convert value to boolean
    JsonValue toBoolean(const JsonValue& value);
    
    // Recursively convert to immutable structure
    JsonValue toImmutable(const JsonValue& value);
    
    // Apply JSONPath query and return result
    JsonPipeline::TransformFunction query(const std::string& jsonPath);
}

/**
 * @brief Common filtering functions
 */
namespace Filters {
    // Check if value is not null
    bool isNotNull(const JsonValue& value);
    
    // Check if value is a number
    bool isNumber(const JsonValue& value);
    
    // Check if value is a string
    bool isString(const JsonValue& value);
    
    // Check if value is an array
    bool isArray(const JsonValue& value);
    
    // Check if value is an object
    bool isObject(const JsonValue& value);
    
    // Check array length
    JsonPipeline::FilterFunction arrayLengthGreaterThan(size_t minLength);
    
    // Check if object contains specified key
    JsonPipeline::FilterFunction objectHasKey(const std::string& key);
}

/**
 * @brief Common aggregation functions
 */
namespace Aggregates {
    // Sum of array elements
    JsonValue sum(const std::vector<JsonValue>& values);
    
    // Average of array elements
    JsonValue average(const std::vector<JsonValue>& values);
    
    // Maximum value in array
    JsonValue max(const std::vector<JsonValue>& values);
    
    // Minimum value in array
    JsonValue min(const std::vector<JsonValue>& values);
    
    // Count of array elements
    JsonValue count(const std::vector<JsonValue>& values);
    
    // Remove duplicates from array
    JsonValue unique(const std::vector<JsonValue>& values);
}

} // namespace JsonStruct