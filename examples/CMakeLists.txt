# examples/CMakeLists.txt
add_executable(example_enhanced_simple examples/example_enhanced_simple.cpp)
target_link_libraries(example_enhanced_simple json_struct)
target_compile_features(example_enhanced_simple PRIVATE cxx_std_17)

add_executable(example_jsonpath_complete examples/example_jsonpath_complete.cpp)
target_link_libraries(example_jsonpath_complete json_struct)
target_compile_features(example_jsonpath_complete PRIVATE cxx_std_17)

add_executable(example_json_filter examples/example_json_filter.cpp)
target_link_libraries(example_json_filter json_struct)
target_compile_features(example_json_filter PRIVATE cxx_std_17)

add_executable(example_json_filter_simple examples/example_json_filter_simple.cpp)
target_link_libraries(example_json_filter_simple json_struct)
target_compile_features(example_json_filter_simple PRIVATE cxx_std_17)

add_executable(streaming_query_example examples/streaming_query_example.cpp)
target_link_libraries(streaming_query_example json_struct)
target_compile_features(streaming_query_example PRIVATE cxx_std_17)

# Version management demo
add_executable(version_demo
    examples/version_demo.cpp
    src/version.cpp
)
target_link_libraries(version_demo json_struct)
target_include_directories(version_demo PRIVATE src)
target_compile_features(version_demo PRIVATE cxx_std_17)

# Create custom target for all examples
add_custom_target(examples
    DEPENDS
        example_enhanced_simple
        example_jsonpath_complete
        example_json_filter
        example_json_filter_simple
        streaming_query_example
        version_demo
)
