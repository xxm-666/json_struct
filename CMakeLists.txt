cmake_minimum_required(VERSION 3.16)
project(JsonStructRegistry LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(src)
include_directories(src/type_registry)
include_directories(src/std_types)
include_directories(src/qt_types)
include_directories(src/json_engine)

# Core library sources
set(CORE_SOURCES
    src/json_engine/json_value.cpp
    src/json_engine/json_path.cpp
)

set(CORE_HEADERS
    src/jsonstruct.h
    src/type_registry/registry_core.h
    src/type_registry/auto_serializer.h
    src/type_registry/field_macros.h
    src/std_types/std_registry.h
    src/std_types/container_registry.h
    # src/std_types/custom_types_example.h
    src/qt_types/qt_registry.h
    src/json_engine/json_value.h
    src/json_engine/json_number.h
    src/json_engine/json_stream_parser.h
    src/json_engine/json_path.h
)

# Create a static library for the core functionality
add_library(jsonstruct_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
target_compile_features(jsonstruct_core PRIVATE cxx_std_17)

# Test executables
add_executable(test_precision_fix_en
    tests/test_precision_fix_en.cpp
)
target_link_libraries(test_precision_fix_en jsonstruct_core)
target_compile_features(test_precision_fix_en PRIVATE cxx_std_17)

add_executable(test_json_parsing
    tests/test_json_parsing.cpp
)
target_link_libraries(test_json_parsing jsonstruct_core)
target_compile_features(test_json_parsing PRIVATE cxx_std_17)

add_executable(test_special_numbers
    tests/test_special_numbers.cpp
)
target_link_libraries(test_special_numbers jsonstruct_core)
target_compile_features(test_special_numbers PRIVATE cxx_std_17)

add_executable(test_error_recovery
    tests/test_error_recovery.cpp
)
target_link_libraries(test_error_recovery jsonstruct_core)
target_compile_features(test_error_recovery PRIVATE cxx_std_17)

add_executable(test_streaming
    tests/test_streaming.cpp
)
target_link_libraries(test_streaming jsonstruct_core)
target_compile_features(test_streaming PRIVATE cxx_std_17)

# Unified JSONPath comprehensive test
add_executable(test_jsonpath_unified
    tests/test_jsonpath.cpp
)
target_link_libraries(test_jsonpath_unified jsonstruct_core)
target_compile_features(test_jsonpath_unified PRIVATE cxx_std_17)

add_executable(test_enhanced_features
    tests/test_enhanced_features.cpp
)
target_link_libraries(test_enhanced_features jsonstruct_core)
target_compile_features(test_enhanced_features PRIVATE cxx_std_17)

add_executable(test_comprehensive_demo
    tests/test_comprehensive_demo.cpp
)
target_link_libraries(test_comprehensive_demo jsonstruct_core)
target_compile_features(test_comprehensive_demo PRIVATE cxx_std_17)

# add_executable(test_std_system
#     tests/test_std_system.cpp
# )
# target_link_libraries(test_std_system jsonstruct_core)
# target_compile_features(test_std_system PRIVATE cxx_std_17)

add_executable(example_enhanced_simple
    examples/example_enhanced_simple.cpp
)
target_link_libraries(example_enhanced_simple jsonstruct_core)
target_compile_features(example_enhanced_simple PRIVATE cxx_std_17)

add_executable(example_jsonpath_complete
    examples/example_jsonpath_complete.cpp
)
target_link_libraries(example_jsonpath_complete jsonstruct_core)
target_compile_features(example_jsonpath_complete PRIVATE cxx_std_17)

# Advanced JSONPath feature test (legacy - use test_jsonpath_unified instead)
# add_executable(test_advanced_jsonpath tests/test_advanced_jsonpath.cpp)
# target_link_libraries(test_advanced_jsonpath jsonstruct_core)
# target_include_directories(test_advanced_jsonpath PRIVATE src)

# Complete JSONPath test (legacy - use test_jsonpath_unified instead)
# add_executable(test_complete_jsonpath test_complete_jsonpath.cpp)
# target_link_libraries(test_complete_jsonpath jsonstruct_core)
# target_include_directories(test_complete_jsonpath PRIVATE src)

# Create custom targets for easy building
add_custom_target(tests
    DEPENDS 
        test_precision_fix_en
        test_json_parsing
        test_special_numbers
        test_error_recovery
        test_streaming
        test_jsonpath_unified
        test_enhanced_features
        test_comprehensive_demo
        test_std_system
)

add_custom_target(examples
    DEPENDS
        example_enhanced_simple
        example_jsonpath_complete
)

add_custom_target(all_targets
    DEPENDS tests examples
)

# Installation
install(TARGETS jsonstruct_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${CORE_HEADERS}
    DESTINATION include/jsonstruct
)

install(FILES src/jsonstruct.h
    DESTINATION include/jsonstruct
)

install(FILES src/type_registry/registry_core.h
             src/type_registry/auto_serializer.h
             src/type_registry/field_macros.h
    DESTINATION include/jsonstruct/type_registry
)

install(FILES src/std_types/std_registry.h
             src/std_types/container_registry.h
             # src/std_types/custom_types_example.h
    DESTINATION include/jsonstruct/std_types
)

install(FILES src/qt_types/qt_registry.h
    DESTINATION include/jsonstruct/qt_types
)

install(FILES src/json_engine/json_value.h
             src/json_engine/json_number.h
             src/json_engine/json_stream_parser.h
             src/json_engine/json_path.h
    DESTINATION include/jsonstruct/json_engine
)
