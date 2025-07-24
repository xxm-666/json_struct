#include "../test_framework/test_framework.h"
#include "../src/json_engine/json_pipeline.h"
#include "../src/json_engine/json_value.h"

using namespace JsonStruct;

TEST(JsonPipelineBasicFilterAndTransform) {
    JsonValue input(JsonValue::ArrayType({
        JsonValue(1), JsonValue("2"), JsonValue(3.14), JsonValue(true), JsonValue(nullptr)
    }));

    JsonPipeline pipeline;
    auto pipelineResult = pipeline
        .filterArray(Filters::isNumber)
        .transformArray(Transforms::toNumber)
        .execute(input);

    ASSERT_EQ(2, pipelineResult.size());
    ASSERT_EQ(1, pipelineResult[0].toDouble());
    ASSERT_EQ(3.14, pipelineResult[1].toDouble());
}

TEST(JsonPipelineAggregateFunctions) {
    JsonValue input(JsonValue::ArrayType({
        JsonValue(1), JsonValue(2), JsonValue(3), JsonValue(4), JsonValue(5)
    }));

    JsonValue sumResult = JsonPipeline()
        .aggregate(Aggregates::sum)
        .execute(input);
    ASSERT_EQ(15, sumResult.toDouble());

    JsonValue avgResult = JsonPipeline()
        .aggregate(Aggregates::average)
        .execute(input);
    ASSERT_EQ(3, avgResult.toDouble());

    JsonValue countResult = JsonPipeline()
        .aggregate(Aggregates::count)
        .execute(input);
    ASSERT_EQ(5, countResult.toDouble());
}

TEST(JsonPipelineBranchOperation) {
    JsonValue input(10);
    JsonPipeline evenPipeline = JsonPipeline().transform([](const JsonValue& v) { return JsonValue(v.toDouble() * 2); });
    JsonPipeline oddPipeline = JsonPipeline().transform([](const JsonValue& v) { return JsonValue(v.toDouble() + 1); });

    auto pipelineResult = JsonPipeline()
        .branch([](const JsonValue& v) { return (int)v.toDouble() % 2 == 0; }, evenPipeline, oddPipeline)
        .execute(input);

    ASSERT_EQ(20, pipelineResult.toDouble());

    input = JsonValue(11);
    pipelineResult = JsonPipeline()
        .branch([](const JsonValue& v) { return (int)v.toDouble() % 2 == 0; }, evenPipeline, oddPipeline)
        .execute(input);
    ASSERT_EQ(12, pipelineResult.toDouble());
}

TEST(JsonPipelineComplexChainedOperations) {
    JsonValue input(JsonValue::ArrayType({
        JsonValue("10"), JsonValue(20), JsonValue("30"), JsonValue(40), JsonValue("not_a_number")
    }));

    auto pipelineResult = JsonPipeline()
        .filterArray([](const JsonValue& v) { return v.isString() || v.isNumber(); })
        .transformArray(Transforms::toNumber)
        .filterArray([](const JsonValue& v) { return v.toDouble() > 15; })
        .aggregateArray(Aggregates::sum)
        .execute(input);

    ASSERT_EQ(90, pipelineResult.toDouble()); // 20 + 30 + 40
}

TEST(JsonPipelineEmptyInputHandling) {
    JsonValue input;
    auto pipelineResult = JsonPipeline()
        .filterArray(Filters::isNumber)
        .transformArray(Transforms::toNumber)
        .aggregateArray(Aggregates::sum)
        .execute(input);

    ASSERT_TRUE(pipelineResult.empty());

    input = JsonValue(JsonValue::ArrayType({}));
    pipelineResult = JsonPipeline()
        .aggregateArray(Aggregates::count)
        .execute(input);
    ASSERT_EQ(0, pipelineResult[0].toDouble());
}

// Boundary test: non-array input
TEST(JsonPipelineNonArrayInput) {
    JsonValue input("string_value");
    auto pipelineResult = JsonPipeline()
        .filterArray(Filters::isNumber)
        .transformArray(Transforms::toNumber)
        .aggregateArray(Aggregates::sum)
        .execute(input);
    ASSERT_TRUE(pipelineResult.empty());
}

// Boundary test: null input
TEST(JsonPipelineNullInput) {
    JsonValue input(nullptr);
    auto pipelineResult = JsonPipeline()
        .filterArray(Filters::isNumber)
        .execute(input);
    ASSERT_TRUE(pipelineResult.empty());
}

// Boundary test: mixed-type array elements
TEST(JsonPipelineMixedTypeArray) {
    JsonValue input(JsonValue::ArrayType({
        JsonValue(1), JsonValue("2"), JsonValue(nullptr), JsonValue(true), JsonValue(JsonValue::ArrayType({JsonValue(3)})), JsonValue(JsonValue::ObjectType({{"key", JsonValue(4)}}))
    }));
    auto pipelineResult = JsonPipeline()
        .filterArray(Filters::isNumber)
        .execute(input);
    // only the number 1 is kept
    ASSERT_EQ(1, pipelineResult.size());
    ASSERT_EQ(1, pipelineResult[0].toDouble());
}

// Boundary test: empty pipeline chain
TEST(JsonPipelineEmptyChain) {
    JsonValue input(JsonValue::ArrayType({JsonValue(1), JsonValue(2)}));
    auto pipelineResult = JsonPipeline().execute(input);
    ASSERT_EQ(input, pipelineResult);
}

// Boundary test: branch always true
TEST(JsonPipelineBranchAlwaysTrue) {
    JsonValue input(5);
    JsonPipeline truePipeline = JsonPipeline().transform([](const JsonValue& v) { return JsonValue(100); });
    JsonPipeline falsePipeline = JsonPipeline().transform([](const JsonValue& v) { return JsonValue(-100); });
    auto pipelineResult = JsonPipeline()
        .branch([](const JsonValue&) { return true; }, truePipeline, falsePipeline)
        .execute(input);
    ASSERT_EQ(100, pipelineResult.toDouble());
}

// Boundary test: branch always false
TEST(JsonPipelineBranchAlwaysFalse) {
    JsonValue input(5);
    JsonPipeline truePipeline = JsonPipeline().transform([](const JsonValue& v) { return JsonValue(100); });
    JsonPipeline falsePipeline = JsonPipeline().transform([](const JsonValue& v) { return JsonValue(-100); });
    auto pipelineResult = JsonPipeline()
        .branch([](const JsonValue&) { return false; }, truePipeline, falsePipeline)
        .execute(input);
    ASSERT_EQ(-100, pipelineResult.toDouble());
}

// Main function to execute tests in this file
int main() {
    return RUN_ALL_TESTS();
}