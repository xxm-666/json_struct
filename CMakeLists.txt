cmake_minimum_required(VERSION 3.16)
project(json_struct LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 定义项目版本号
set(PROJECT_VERSION 1.0.0)

# Include directories
include_directories(src)
include_directories(src/type_registry)
include_directories(src/std_types)
include_directories(src/qt_types)
include_directories(src/json_engine)

# Core library sources
set(CORE_SOURCES
    src/json_engine/json_value.cpp
    src/json_engine/json_query_generator.cpp
    src/json_engine/json_path.cpp
    src/json_engine/json_filter.cpp
)
set(CORE_HEADERS
    src/jsonstruct.h
    src/type_registry/registry_core.h
    src/type_registry/auto_serializer.h
    src/type_registry/field_macros.h
    src/std_types/std_registry.h
    src/std_types/container_registry.h
    src/qt_types/qt_registry.h
    src/json_engine/json_value.h
    src/json_engine/json_number.h
    src/json_engine/json_stream_parser.h
    src/json_engine/json_path.h
    src/json_engine/json_filter.h
)

add_library(json_struct STATIC ${CORE_SOURCES} ${CORE_HEADERS})
target_compile_features(json_struct PRIVATE cxx_std_17)

# 选项控制是否编译测试和示例
option(BUILD_TESTS "Build test targets" OFF)
option(BUILD_EXAMPLES "Build example targets" OFF)

if(BUILD_TESTS)
    include(tests/CMakeLists.txt)
endif()

if(BUILD_EXAMPLES)
    include(examples/CMakeLists.txt)
endif()

# 安装部分保持原样
install(TARGETS json_struct
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# install(FILES ${CORE_HEADERS}
#     DESTINATION include/jsonstruct
# )

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

# 修复导出集名称问题
install(TARGETS json_struct
        EXPORT json_structTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

export(EXPORT json_structTargets
       FILE json_structTargets.cmake
       NAMESPACE json_struct::)

# 生成配置文件
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/json_structConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_file(json_structConfig.cmake.in
               "${CMAKE_CURRENT_BINARY_DIR}/json_structConfig.cmake"
               @ONLY)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/json_structConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/json_structConfigVersion.cmake"
    DESTINATION lib/cmake/json_struct
)

# 安装导出文件
install(EXPORT json_structTargets
        FILE json_structTargets.cmake
        NAMESPACE json_struct::
        DESTINATION lib/cmake/json_struct)
