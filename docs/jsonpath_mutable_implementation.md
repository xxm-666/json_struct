# JSONPath可修改操作实现总结

## 实现的功能

### 1. 新增的结构体
- `MutableQueryResult`: 支持可修改操作的查询结果结构体
  - `std::vector<std::reference_wrapper<JsonStruct::JsonValue>> values`: 可修改的值引用列表
  - `std::vector<std::string> paths`: 对应的路径
  - `first()`: 获取第一个可修改值的引用

### 2. JsonPath类新增方法
- `evaluateMutable(JsonValue& root)`: 评估JSONPath并返回可修改的结果
- `selectFirstMutable(JsonValue& root)`: 选择第一个匹配的可修改值
- `selectAllMutable(JsonValue& root)`: 选择所有匹配的可修改值

### 3. 便利函数
- `jsonvalue_jsonpath::queryMutable()`: 可修改的查询操作
- `jsonvalue_jsonpath::selectFirstMutable()`: 选择第一个可修改值
- `jsonvalue_jsonpath::selectAllMutable()`: 选择所有可修改值

### 4. 支持的操作类型

#### 单值修改
```cpp
auto first_price = selectFirstMutable(root, "$.store.book[0].price");
if (first_price.has_value()) {
    first_price->get() = 12.0;  // 修改单个值
}
```

#### 批量修改
```cpp
auto all_prices = selectAllMutable(root, "$.products[*].price");
for (auto& price_ref : all_prices) {
    auto& price = price_ref.get();
    price = price.toDouble() * 0.9;  // 批量应用10%折扣
}
```

#### 递归修改
```cpp
auto all_status = selectAllMutable(root, "$..status");
for (auto& status_ref : all_status) {
    status_ref.get() = "completed";  // 修改所有嵌套的status字段
}
```

#### 数组切片修改
```cpp
auto slice = selectAllMutable(root, "$.numbers[1:4]");
for (auto& num_ref : slice) {
    num_ref.get() = num_ref.get().toInt() * 10;  // 修改数组片段
}
```

#### 条件修改
```cpp
auto all_items = selectAllMutable(root, "$.inventory[*]");
for (auto& item_ref : all_items) {
    auto& item = item_ref.get();
    if (item["quantity"].toInt() == 0) {
        item["quantity"] = -1;  // 条件性修改
    }
}
```

## 实现的辅助方法

### JsonPath类的可修改辅助方法
- `evaluateNodeMutable()`: 可修改节点评估
- `evaluatePropertyMutable()`: 可修改属性评估
- `evaluateIndexMutable()`: 可修改索引评估
- `evaluateSliceMutable()`: 可修改切片评估
- `evaluateWildcardMutable()`: 可修改通配符评估
- `evaluateRecursiveMutable()`: 可修改递归评估
- `evaluateFilterMutable()`: 可修改过滤器评估
- `collectRecursiveMutable()`: 可修改递归收集
- `collectRecursivePropertyMutable()`: 可修改递归属性收集

## 技术特点

1. **类型安全**: 使用`std::reference_wrapper`确保引用的有效性
2. **一致性**: 可修改操作的API与只读操作保持一致
3. **灵活性**: 支持所有JSONPath查询模式的可修改版本
4. **性能**: 直接修改原始数据，避免不必要的复制

## 测试覆盖

实现了全面的测试用例：
- `test_single_value_modification()`: 单值修改测试
- `test_batch_modification()`: 批量修改测试
- `test_nested_modification()`: 嵌套修改测试
- `test_array_modification()`: 数组修改测试
- `test_conditional_modification()`: 条件修改测试
- `test_query_result_modification()`: 查询结果修改测试

## 使用方法

### 基本用法
```cpp
#include "json_path.h"
using namespace jsonvalue_jsonpath;

JsonValue root;
// ... 初始化JSON数据 ...

// 单值修改
auto value = selectFirstMutable(root, "$.path.to.value");
if (value.has_value()) {
    value->get() = newValue;
}

// 批量修改
auto values = selectAllMutable(root, "$.path.to.values[*]");
for (auto& val : values) {
    val.get() = transform(val.get());
}
```

### 查询结果修改
```cpp
auto result = queryMutable(root, "$.config.*");
for (auto& value_ref : result.values) {
    auto& value = value_ref.get();
    if (value.isNumber()) {
        value = value.toDouble() * 2;
    }
}
```

## 注意事项

1. 确保在修改前JSON数据结构是可修改的（非const）
2. 使用正确的JsonValue访问方法（toInt(), toDouble(), toString()等）
3. 修改操作直接作用于原始数据，无需额外的保存步骤
4. 支持所有标准JSONPath表达式的可修改版本

这个实现提供了完整的JSONPath可修改操作支持，允许用户方便地查询和修改JSON数据结构。
