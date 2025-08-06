cmake_minimum_required(VERSION 3.16)
project(json_struct LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== 版本管理 =====
# 定义项目版本号 (语义化版本控制: MAJOR.MINOR.PATCH)
set(JSON_STRUCT_VERSION_MAJOR 1)
set(JSON_STRUCT_VERSION_MINOR 2)
set(JSON_STRUCT_VERSION_PATCH 0)
set(JSON_STRUCT_VERSION_SUFFIX "dev")  # 空字符串表示正式版本

# 计算完整版本号 
if(JSON_STRUCT_VERSION_SUFFIX STREQUAL "")
    set(PROJECT_VERSION "${JSON_STRUCT_VERSION_MAJOR}.${JSON_STRUCT_VERSION_MINOR}.${JSON_STRUCT_VERSION_PATCH}")
else()
    set(PROJECT_VERSION "${JSON_STRUCT_VERSION_MAJOR}.${JSON_STRUCT_VERSION_MINOR}.${JSON_STRUCT_VERSION_PATCH}-${JSON_STRUCT_VERSION_SUFFIX}")
endif()

# 获取构建时间
string(TIMESTAMP JSON_STRUCT_BUILD_DATE "%Y-%m-%d %H:%M:%S UTC" UTC)

# 获取Git信息（如果可用）
find_package(Git QUIET)
if(GIT_FOUND)
    # 获取当前提交哈希
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE JSON_STRUCT_GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    # 获取当前分支
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE JSON_STRUCT_GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
else()
    set(JSON_STRUCT_GIT_COMMIT "unknown")
    set(JSON_STRUCT_GIT_BRANCH "unknown")
endif()


# 生成版本头文件到构建目录，避免只读源目录问题
set(GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GENERATED_INCLUDE_DIR}")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in"
    "${GENERATED_INCLUDE_DIR}/version.h"
    @ONLY
)

# 添加生成目录到头文件搜索路径
include_directories(BEFORE "${GENERATED_INCLUDE_DIR}")

# 显示版本信息
message(STATUS "JsonStruct Version: ${PROJECT_VERSION}")
message(STATUS "Build Date: ${JSON_STRUCT_BUILD_DATE}")
message(STATUS "Git Commit: ${JSON_STRUCT_GIT_COMMIT}")
message(STATUS "Git Branch: ${JSON_STRUCT_GIT_BRANCH}")

# Include directories
include_directories(src)
include_directories(src/type_registry)
include_directories(src/std_types)
include_directories(src/json_engine)

# Core library sources
set(CORE_SOURCES
    src/json_engine/json_value.cpp
    src/json_engine/json_query_generator.cpp
    src/json_engine/json_path.cpp
    src/json_engine/internal/json_path_tokenizer.cpp
    src/json_engine/internal/json_path_parser.cpp
    src/json_engine/internal/json_path_simple_evaluator.cpp
    src/json_engine/internal/json_path_filter_evaluator.cpp
    src/json_engine/internal/json_path_advanced_evaluator.cpp
    src/json_engine/json_filter.cpp
    src/json_engine/json_patch.cpp
    src/json_engine/lazy_query_generator.cpp
    src/version.cpp
    src/json_engine/json_pipeline.cpp
)
set(CORE_HEADERS
    src/jsonstruct.h
    src/type_registry/registry_core.h
    src/type_registry/auto_serializer.h
    src/type_registry/field_macros.h
    src/std_types/std_registry.h
    src/json_engine/json_value.h
    src/json_engine/json_number.h
    src/json_engine/json_stream_parser.h
    src/json_engine/json_path.h
    src/json_engine/internal/json_path_tokenizer.h
    src/json_engine/internal/json_path_parser.h
    src/json_engine/internal/json_path_simple_evaluator.h
    src/json_engine/internal/json_path_filter_evaluator.h
    src/json_engine/internal/json_path_advanced_evaluator.h
    src/json_engine/json_filter.h
    src/json_engine/json_pipeline.h
    src/json_engine/lazy_query_generator.h
)

# 可选的Qt支持
option(ENABLE_QT_SUPPORT "Enable Qt support" OFF)

if(ENABLE_QT_SUPPORT)
    find_package(Qt6 COMPONENTS Core Widgets)
    if(Qt6_FOUND)
        add_definitions(-DQT_CORE_LIB)
        # 添加Qt相关的源文件
        list(APPEND CORE_SOURCES
            src/qt_types/qt_ultimate_registry.cpp
            src/qt_types/qt_common.cpp
        )
        list(APPEND CORE_HEADERS
            src/qt_types/qt_registry.h
            src/qt_types/qt_ultimate_registry.h
            src/qt_types/qt_common.h
        )
        # 添加Qt包含目录
        include_directories(src/qt_types)
        message(STATUS "Qt6 found - Qt support enabled")
    else()
        message(WARNING "Qt6 not found - Qt support disabled")
    endif()
endif()

# 添加INTERFACE头文件路径
add_library(json_struct STATIC ${CORE_SOURCES} ${CORE_HEADERS})
target_include_directories(json_struct INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include/jsonstruct>
)

if(ENABLE_QT_SUPPORT AND Qt6_FOUND)
    target_link_libraries(json_struct PUBLIC Qt6::Core Qt6::Widgets)
    target_include_directories(json_struct PUBLIC ${Qt6Core_INCLUDE_DIRS})
endif()

target_compile_features(json_struct PRIVATE cxx_std_17)

# 选项控制是否编译测试和示例
option(BUILD_TESTS "Build test targets" ON)
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

install(FILES src/jsonstruct.h
        "${GENERATED_INCLUDE_DIR}/version.h"
    DESTINATION include/jsonstruct
)

install(FILES src/type_registry/registry_core.h
             src/type_registry/auto_serializer.h
             src/type_registry/field_macros.h
    DESTINATION include/jsonstruct/type_registry
)

install(FILES src/std_types/std_registry.h
    DESTINATION include/jsonstruct/std_types
)

# Qt类型支持头文件
if(ENABLE_QT_SUPPORT)
    find_package(Qt6 COMPONENTS Core Widgets QUIET)
    if(Qt6_FOUND)
        install(FILES src/qt_types/qt_registry.h
                     src/qt_types/qt_common.h
                     src/qt_types/qt_ultimate_registry.h
            DESTINATION include/jsonstruct/qt_types
        )
    endif()
endif()

install(FILES src/json_engine/json_value.h
             src/json_engine/json_number.h
             src/json_engine/json_stream_parser.h
             src/json_engine/json_path.h
             src/json_engine/json_error.h
             src/json_engine/json_filter.h
             src/json_engine/json_pipeline.h
             src/json_engine/lazy_query_generator.h
             src/json_engine/json_patch.h
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