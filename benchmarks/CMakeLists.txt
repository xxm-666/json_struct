cmake_minimum_required(VERSION 3.16)
project(JsonStructBenchmarks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 显示项目信息
message(STATUS "==========================================")
message(STATUS "JsonStruct Performance Benchmark Suite")
message(STATUS "==========================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

# ===== 第三方库依赖管理 =====
include(FetchContent)

# 选项：是否构建与真实JSON库的对比测试
option(BUILD_REAL_JSON_COMPARISON "Build comparison tests with real JSON libraries" ON)

if(BUILD_REAL_JSON_COMPARISON)
    message(STATUS "Fetching JSON libraries for benchmark comparison...")
    
    # nlohmann/json
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
    )
    
    # RapidJSON
    FetchContent_Declare(
        rapidjson
        GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
        GIT_TAG v1.1.0
        GIT_SHALLOW TRUE
    )
    
    # jsoncpp
    FetchContent_Declare(
        jsoncpp
        GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
        GIT_TAG 1.9.5
        GIT_SHALLOW TRUE
    )
    
    # 设置jsoncpp选项
    set(JSONCPP_WITH_TESTS OFF CACHE BOOL "Build jsoncpp tests" FORCE)
    set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE BOOL "Build jsoncpp post build unittest" FORCE)
    set(JSONCPP_WITH_WARNING_AS_ERROR OFF CACHE BOOL "Treat warnings as errors" FORCE)
    set(JSONCPP_WITH_PKGCONFIG_SUPPORT OFF CACHE BOOL "Build jsoncpp pkg-config support" FORCE)
    set(JSONCPP_WITH_CMAKE_PACKAGE OFF CACHE BOOL "Build jsoncpp cmake package" FORCE)
    
    # 获取所有库
    FetchContent_MakeAvailable(nlohmann_json rapidjson jsoncpp)
    
    message(STATUS "JSON libraries fetched successfully")
else()
    message(STATUS "Skipping real JSON libraries - using mock implementations")
endif()

# ===== JsonStruct 库路径设置 =====
set(JSON_STRUCT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(JSON_STRUCT_SRC "${JSON_STRUCT_ROOT}/src")
set(JSON_STRUCT_TESTS "${JSON_STRUCT_ROOT}/tests")
set(JSON_STRUCT_TEST_FRAMEWORK "${JSON_STRUCT_ROOT}/test_framework")

# 包含目录
include_directories(${JSON_STRUCT_SRC})
include_directories(${JSON_STRUCT_SRC}/type_registry)
include_directories(${JSON_STRUCT_SRC}/std_types)
include_directories(${JSON_STRUCT_SRC}/qt_types)
include_directories(${JSON_STRUCT_SRC}/json_engine)
include_directories(${JSON_STRUCT_TEST_FRAMEWORK})

# 查找系统依赖
find_package(Threads REQUIRED)

# ===== JsonStruct 核心库 =====
set(JSONSTRUCT_SOURCES
    ${JSON_STRUCT_SRC}/json_engine/json_value.cpp
    ${JSON_STRUCT_SRC}/json_engine/json_query_generator.cpp
    ${JSON_STRUCT_SRC}/json_engine/json_path.cpp
    ${JSON_STRUCT_SRC}/json_engine/json_filter.cpp
    ${JSON_STRUCT_SRC}/version.cpp
    ${JSON_STRUCT_SRC}/qt_types/qt_ultimate_registry.cpp
    ${JSON_STRUCT_SRC}/qt_types/qt_common.cpp
)

# 创建JsonStruct静态库
add_library(json_struct_benchmark STATIC ${JSONSTRUCT_SOURCES})
target_include_directories(json_struct_benchmark PUBLIC ${JSON_STRUCT_SRC})
target_compile_features(json_struct_benchmark PUBLIC cxx_std_17)

# Qt支持检查
option(ENABLE_QT_SUPPORT_BENCHMARK "Enable Qt support for benchmarks" OFF)
if(ENABLE_QT_SUPPORT_BENCHMARK)
    find_package(Qt6 COMPONENTS Core Widgets QUIET)
    if(Qt6_FOUND)
        add_definitions(-DQT_CORE_LIB)
        target_link_libraries(json_struct_benchmark PUBLIC Qt6::Core Qt6::Widgets)
        message(STATUS "Qt6 support enabled for benchmarks")
    else()
        message(STATUS "Qt6 not found - Qt support disabled for benchmarks")
    endif()
endif()

# ===== 测试框架 =====
add_library(test_framework_benchmark STATIC
    ${JSON_STRUCT_TEST_FRAMEWORK}/test_framework.cpp
    ${JSON_STRUCT_TEST_FRAMEWORK}/test_framework.h
)
target_include_directories(test_framework_benchmark PUBLIC ${JSON_STRUCT_TEST_FRAMEWORK})
target_compile_features(test_framework_benchmark PUBLIC cxx_std_17)

# ===== 性能测试可执行文件 =====

# 基础性能测试
add_executable(performance_benchmark 
    ${JSON_STRUCT_TESTS}/performance_benchmark.cpp
)
target_link_libraries(performance_benchmark PRIVATE 
    test_framework_benchmark 
    json_struct_benchmark 
    Threads::Threads
)
target_compile_features(performance_benchmark PRIVATE cxx_std_17)

# 内存使用测试
add_executable(memory_benchmark 
    ${JSON_STRUCT_TESTS}/memory_benchmark.cpp
)
target_link_libraries(memory_benchmark PRIVATE 
    test_framework_benchmark 
    json_struct_benchmark 
    Threads::Threads
)
target_compile_features(memory_benchmark PRIVATE cxx_std_17)

# 性能报告生成器
add_executable(performance_reporter 
    ${JSON_STRUCT_TESTS}/performance_reporter.cpp
)
target_link_libraries(performance_reporter PRIVATE 
    test_framework_benchmark 
    json_struct_benchmark 
    Threads::Threads
)
target_compile_features(performance_reporter PRIVATE cxx_std_17)

# 对比测试（根据是否有真实库决定链接）
add_executable(comparison_benchmark 
    ${JSON_STRUCT_TESTS}/comparison_benchmark.cpp
)
target_link_libraries(comparison_benchmark PRIVATE 
    test_framework_benchmark 
    json_struct_benchmark 
    Threads::Threads
)
target_compile_features(comparison_benchmark PRIVATE cxx_std_17)

# 如果启用了真实库对比，添加第三方库链接
if(BUILD_REAL_JSON_COMPARISON)
    if(TARGET nlohmann_json)
        target_link_libraries(comparison_benchmark PRIVATE nlohmann_json)
        target_compile_definitions(comparison_benchmark PRIVATE HAVE_NLOHMANN_JSON)
        message(STATUS "comparison_benchmark: nlohmann_json linked")
    endif()
    
    if(TARGET rapidjson)
        target_link_libraries(comparison_benchmark PRIVATE rapidjson)
        target_compile_definitions(comparison_benchmark PRIVATE HAVE_RAPIDJSON)
        message(STATUS "comparison_benchmark: rapidjson linked")
    endif()
    
    if(TARGET jsoncpp_lib)
        target_link_libraries(comparison_benchmark PRIVATE jsoncpp_lib)
        target_compile_definitions(comparison_benchmark PRIVATE HAVE_JSONCPP)
        message(STATUS "comparison_benchmark: jsoncpp linked")
    endif()
    
    target_compile_definitions(comparison_benchmark PRIVATE USE_REAL_JSON_LIBRARIES)
    message(STATUS "comparison_benchmark: Using real JSON libraries")
else()
    message(STATUS "comparison_benchmark: Using mock JSON libraries")
endif()

# 简化的对比测试（不需要第三方库）
add_executable(comparison_benchmark_simple 
    ${JSON_STRUCT_TESTS}/comparison_benchmark_simple.cpp
)
target_link_libraries(comparison_benchmark_simple PRIVATE 
    test_framework_benchmark 
    json_struct_benchmark 
    Threads::Threads
)
target_compile_features(comparison_benchmark_simple PRIVATE cxx_std_17)

# ===== 编译选项 =====
if(MSVC)
    target_compile_options(json_struct_benchmark PRIVATE /W4 /O2)
    target_compile_options(performance_benchmark PRIVATE /W4 /O2)
    target_compile_options(memory_benchmark PRIVATE /W4 /O2)
    target_compile_options(comparison_benchmark PRIVATE /W4 /O2)
    target_compile_options(comparison_benchmark_simple PRIVATE /W4 /O2)
    target_compile_options(performance_reporter PRIVATE /W4 /O2)
else()
    target_compile_options(json_struct_benchmark PRIVATE -Wall -Wextra -O3)
    target_compile_options(performance_benchmark PRIVATE -Wall -Wextra -O3)
    target_compile_options(memory_benchmark PRIVATE -Wall -Wextra -O3)
    target_compile_options(comparison_benchmark PRIVATE -Wall -Wextra -O3)
    target_compile_options(comparison_benchmark_simple PRIVATE -Wall -Wextra -O3)
    target_compile_options(performance_reporter PRIVATE -Wall -Wextra -O3)
endif()

# ===== 自定义目标 =====

# 构建所有性能测试
add_custom_target(build_all_benchmarks
    DEPENDS performance_benchmark memory_benchmark comparison_benchmark 
            comparison_benchmark_simple performance_reporter
    COMMENT "Building all performance benchmark executables"
)

# 运行基础性能测试
add_custom_target(run_performance_tests
    COMMAND performance_benchmark
    COMMAND memory_benchmark
    COMMAND performance_reporter
    DEPENDS performance_benchmark memory_benchmark performance_reporter
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running basic performance tests"
)

# 运行对比测试
add_custom_target(run_comparison_tests
    COMMAND comparison_benchmark_simple
    COMMAND comparison_benchmark
    DEPENDS comparison_benchmark_simple comparison_benchmark
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running comparison tests"
)

# 运行所有性能测试
add_custom_target(run_all_benchmarks
    COMMAND performance_benchmark
    COMMAND memory_benchmark
    COMMAND comparison_benchmark_simple
    COMMAND comparison_benchmark
    COMMAND performance_reporter
    DEPENDS build_all_benchmarks
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running complete performance benchmark suite"
)

# 清理结果文件
add_custom_target(clean_benchmark_results
    COMMAND ${CMAKE_COMMAND} -E remove -f performance_results.json
    COMMAND ${CMAKE_COMMAND} -E remove -f benchmark_results.txt
    COMMAND ${CMAKE_COMMAND} -E remove -f memory_results.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Cleaning benchmark result files"
)

# ===== 安装规则 =====
install(TARGETS performance_benchmark memory_benchmark comparison_benchmark 
               comparison_benchmark_simple performance_reporter
        DESTINATION bin
        COMPONENT benchmarks)

# ===== 总结信息 =====
message(STATUS "==========================================")
message(STATUS "Benchmark Configuration Summary:")
message(STATUS "Real JSON libraries: ${BUILD_REAL_JSON_COMPARISON}")
message(STATUS "Qt support: ${ENABLE_QT_SUPPORT_BENCHMARK}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "==========================================")
message(STATUS "Available targets:")
message(STATUS "  build_all_benchmarks   - Build all benchmark executables")
message(STATUS "  run_performance_tests  - Run basic performance tests")
message(STATUS "  run_comparison_tests   - Run comparison tests")
message(STATUS "  run_all_benchmarks     - Run all benchmark tests")
message(STATUS "  clean_benchmark_results - Clean result files")
message(STATUS "==========================================")
