# examples/CMakeLists.txt
add_executable(example_enhanced_simple examples/example_enhanced_simple.cpp)
target_link_libraries(example_enhanced_simple jsonstruct_core)
target_compile_features(example_enhanced_simple PRIVATE cxx_std_17)

add_executable(example_jsonpath_complete examples/example_jsonpath_complete.cpp)
target_link_libraries(example_jsonpath_complete jsonstruct_core)
target_compile_features(example_jsonpath_complete PRIVATE cxx_std_17)

add_executable(example_json_filter examples/example_json_filter.cpp)
target_link_libraries(example_json_filter jsonstruct_core)
target_compile_features(example_json_filter PRIVATE cxx_std_17)

add_executable(example_json_filter_simple examples/example_json_filter_simple.cpp)
target_link_libraries(example_json_filter_simple jsonstruct_core)
target_compile_features(example_json_filter_simple PRIVATE cxx_std_17)

# Create custom target for all examples
add_custom_target(examples
    DEPENDS
        example_enhanced_simple
        example_jsonpath_complete
        example_json_filter
        example_json_filter_simple
)
