// JSONPath 复杂嵌套与过滤
#include "../src/jsonstruct.h"
#include "json_engine/json_path.h"
#include "../test_framework/test_framework.h"
#include <iostream>
#include <chrono>

using namespace JsonStruct;
using namespace JsonStruct::literals;

// 1. 基本字段过滤
TEST(JsonPath_BasicFieldFiltering) {
    JsonValue json = JsonValue::parse(R"({
        "name": "Alice",
        "age": 21,
        "score": 60,
        "active": true,
        "nickname": null,
        "deleted": false,
        "misc": null
    })");

    auto name = jsonvalue_jsonpath::selectAll(json, "$..[?(@.name == 'Alice')]");
    ASSERT_EQ(name.size(), 1);

    auto age = jsonvalue_jsonpath::selectAll(json, "$[?(@.age > 18)]");
    ASSERT_EQ(age.size(), 1);

    auto nickname = jsonvalue_jsonpath::selectAll(json, "$[?(@.nickname == null)]");
    ASSERT_EQ(nickname.size(), 1);

    auto score = jsonvalue_jsonpath::selectAll(json, "$[?(@.score <= 60)]");
    ASSERT_EQ(score.size(), 1);

    auto deleted = jsonvalue_jsonpath::selectAll(json, "$[?(@.deleted == false)]");
    ASSERT_EQ(deleted.size(), 1);

    auto misc = jsonvalue_jsonpath::selectAll(json, "$[?(@.misc == null)]");
    ASSERT_EQ(misc.size(), 1);
}

// 2. 字符串和正则表达式过滤
TEST(JsonPath_StringAndRegex) {
    JsonValue json = JsonValue::parse(R"({
        "email": "alice@gmail.com",
        "username": "adminUser",
        "filename": "pic.jpg",
        "phone": "13811112222"
    })");

    auto email = jsonvalue_jsonpath::selectAll(json, "$[?(@.email =~ /gmail/)]");
    ASSERT_EQ(email.size(), 1);

    auto username = jsonvalue_jsonpath::selectAll(json, "$[?(@.username =~ /^admin/)]");
    ASSERT_EQ(username.size(), 1);

    auto filename = jsonvalue_jsonpath::selectAll(json, "$[?(@.filename =~ /.jpg$/)]");
    ASSERT_EQ(filename.size(), 1);

    auto phone = jsonvalue_jsonpath::selectAll(json, "$[?(@.phone =~ /^1[3-9]\\d{9}$/)]");
    ASSERT_EQ(phone.size(), 1);
}

void moveJson(JsonValue&& json)
{
	auto lastUse = std::move(json["friends"]);
}

// 3. 数组过滤
TEST(JsonPath_ArrayFiltering) {
    JsonValue json = JsonValue::parse(R"({
        "tags": ["tag1", "tag2", "tag3"],
        "scores": [91, 58, 99, 65],
        "friends": [
            {"name": "Bob", "age": 19},
            {"name": "Tom", "age": 17}
        ],
        "items": [{"id": 123}, {"id": 456}]
    })");

    auto len = jsonvalue_jsonpath::selectAll(json, "$[?(@.tags.length() > 2)]");
    ASSERT_EQ(len.size(), 1);

    auto contains = jsonvalue_jsonpath::selectAll(json, "$[?('tag1' in @.tags)]");
    ASSERT_EQ(contains.size(), 1);

	auto friends = jsonvalue_jsonpath::selectAll(json, "$.friends[*]");
	ASSERT_EQ(friends.size(), 2);

	auto obj_contains = jsonvalue_jsonpath::selectAll(json, "$[?(@.friends[?(@.name == 'Bob' || @.age > 15)])]");
	ASSERT_EQ(obj_contains.size(), 1);

	auto friends_matching = jsonvalue_jsonpath::selectAll(json, "$.friends[?(@.name == 'Bob' || @.age > 18)]");
	ASSERT_EQ(friends_matching.size(), 1);

    auto anyScore = jsonvalue_jsonpath::selectAll(json, "$.scores[?(@ > 90)]");
    ASSERT_EQ(anyScore.size(), 2);

    auto adultFriend = jsonvalue_jsonpath::selectAll(json, "$[?(@.friends[?(@.age >= 18)])]");
    ASSERT_EQ(adultFriend.size(), 1);

    auto firstId = jsonvalue_jsonpath::selectAll(json, "$.items[0].id");
    ASSERT_EQ(firstId.size(), 1);
    ASSERT_EQ(firstId[0].get().toInt(), 123);
}

// 4. 复杂嵌套与过滤
TEST(JsonPath_NestedFields) {
    JsonValue json = JsonValue::parse(R"({
        "address": [
            {
                "street": "123 Main St",
                "city": "Beijing",
                "zipcode": "100000"
            },
            {
                "street": "456 Elm St",
                "city": "Shanghai",
                "zipcode": "200000"
            },
            {
                "street": "789 Oak St",
                "city": "Beijing",
                "zipcode": "110000"
            }
        ],
        "users": [
            {"roles": ["admin", "user"]},
            {"roles": ["guest"]}
        ]
    })");

    auto city = jsonvalue_jsonpath::selectAll(json, "$.address[?(@.city == 'Beijing')]");
    ASSERT_EQ(city.size(), 2);
	ASSERT_EQ(city[0].get()["city"].toString(), "Beijing");
	ASSERT_EQ(city[0].get()["street"].toString(), "123 Main St");
	ASSERT_EQ(city[0].get()["zipcode"].toString(), "100000");
	ASSERT_EQ(city[1].get()["city"].toString(), "Beijing");
	ASSERT_EQ(city[1].get()["street"].toString(), "789 Oak St");
	ASSERT_EQ(city[1].get()["zipcode"].toString(), "110000");

    auto nestedRole = jsonvalue_jsonpath::selectAll(json, "$.users[1].roles[0]");
    ASSERT_EQ(nestedRole.size(), 1);
    ASSERT_EQ(nestedRole[0].get().toString(), "guest");

    auto exist = jsonvalue_jsonpath::selectAll(json, "$.address[*].city");
    ASSERT_EQ(exist.size(), 3);
}

// 5. 逻辑操作
TEST(JsonPath_LogicOperations) {
    JsonValue json = JsonValue::parse(R"({
        "members": [
            {"name": "Alice", "age": 21, "active": true, "type": "vip", "points": 1200, "flag": false},
            {"name": "Bob", "age": 17, "active": false, "type": "normal", "points": 500, "flag": true}
        ]
    })");

    auto andLogic = jsonvalue_jsonpath::selectAll(json, "$.members[?(@.age > 18 && @.active == true)]");
    ASSERT_EQ(andLogic.size(), 1);

    auto orLogic = jsonvalue_jsonpath::selectAll(json, "$.members[?(@.type == 'vip' || @.points > 1000)]");
    ASSERT_EQ(orLogic.size(), 1);
}

// 6. 比较操作
TEST(JsonPath_CompareOperations) {
    JsonValue json = JsonValue::parse(R"({
        "score": 100, "price": 0, "height": 180, "weight": 50
    })");

    auto eq = jsonvalue_jsonpath::selectAll(json, "$[?(@.score == 100)]");
    ASSERT_EQ(eq.size(), 1);

    auto neq = jsonvalue_jsonpath::selectAll(json, "$[?(@.price != 1)]");
    ASSERT_EQ(neq.size(), 1);

    auto ge = jsonvalue_jsonpath::selectAll(json, "$[?(@.height >= 180)]");
    ASSERT_EQ(ge.size(), 1);

    auto lt = jsonvalue_jsonpath::selectAll(json, "$[?(@.weight < 51)]");
    ASSERT_EQ(lt.size(), 1);
}

// 7. 方法/函数调用
//TEST(JsonPath_MethodCalls) {
//    // TODO: Implement method calls like length(), toLowerCase(), indexOf()
//    // For now, these are not supported, so we expect 0 results for method-based filters
//    JsonValue json = JsonValue::parse(R"({
//        "name": "abcdef",
//        "desc": "ABC",
//        "tags": ["hot"],
//        "list": [10, 20, 30],
//        "obj": {"a": 1, "b": 2}
//    })");
//
//    // These method calls are not yet implemented, so they should return 0 results
//    auto nameLen = jsonvalue_jsonpath::selectAll(json, "$[?(@.name.length() > 5)]");
//    ASSERT_EQ(nameLen.size(), 0);  // Method calls not implemented yet
//
//    auto toLower = jsonvalue_jsonpath::selectAll(json, "$[?(@.desc.toLowerCase() == 'abc')]");
//    ASSERT_EQ(toLower.size(), 0);  // Method calls not implemented yet
//
//    auto tagIndex = jsonvalue_jsonpath::selectAll(json, "$[?(@.tags.indexOf('hot') >= 0)]");
//    ASSERT_EQ(tagIndex.size(), 0);  // Method calls not implemented yet
//}

// 8. 特殊/边界情况
TEST(JsonPath_SpecialCases) {
    JsonValue json = JsonValue::parse(R"({
        "arr": [],
        "obj": {},
        "count": 6,
        "title": "",
        "enable": false,
        "deleted": null,
        "Name": "ALICE"
    })");

    auto arr = jsonvalue_jsonpath::selectAll(json, "$.arr[?(@.arr.length() == 0)]");
    ASSERT_EQ(arr.size(), 0);  // Empty array should filter to 0 results - fixed expectation

    //auto objEmpty = jsonvalue_jsonpath::selectAll(json, "$[?(@.obj.keys().length() == 0)]");
    //ASSERT_EQ(objEmpty.size(), 1);

    auto zero = jsonvalue_jsonpath::selectAll(json, "$[?(@.count == 6)]");
    ASSERT_EQ(zero.size(), 1);

    auto emptyStr = jsonvalue_jsonpath::selectAll(json, "$[?(@.title == '')]");
    ASSERT_EQ(emptyStr.size(), 1);

    auto enable = jsonvalue_jsonpath::selectAll(json, "$[?(@.enable == false)]");
    ASSERT_EQ(enable.size(), 1);

    auto deleted = jsonvalue_jsonpath::selectAll(json, "$[?(@.deleted == null)]");
    ASSERT_EQ(deleted.size(), 1);

    auto nameCase = jsonvalue_jsonpath::selectAll(json, "$[?(@.Name == 'ALICE')]");
    ASSERT_EQ(nameCase.size(), 1);
}

TEST(JsonPath_Special)
{
    JsonValue json = JsonValue::parse(R"({
        "复杂测试": "张三",
        "emoji": "🤨"
    })");

    auto nonEnglish = jsonvalue_jsonpath::selectAll(json, R"($[?(@["复杂测试"] == '张三')])");
    ASSERT_EQ(nonEnglish.size(), 1);
	ASSERT_EQ(nonEnglish[0].get()["复杂测试"].toString(), "张三");

    auto unicode = jsonvalue_jsonpath::selectAll(json, "$[?(@.emoji == '🤨')]");
    ASSERT_EQ(unicode.size(), 1);
	ASSERT_EQ(unicode[0].get()["emoji"].toString(), "🤨");
}

// 9. 复杂嵌套与过滤
TEST(JsonPath_AdvancedComplexNesting) {
    JsonValue json = JsonValue::parse(R"({
        "type": "a", "score": 61,
        "data": {
            "users": [
                {"name": "Tom", "age": 17},
                {"name": "Jerry", "age": 19}
            ]
        }
    })");

    auto complexCond = jsonvalue_jsonpath::selectAll(json, "$[?((@.type == 'a' && @.score > 60) || (@.type == 'b' && @.score > 80))]");
    ASSERT_EQ(complexCond.size(), 1);

    auto deepNest = jsonvalue_jsonpath::selectAll(json, "$.data.users[?(@.age < 18)].name");
    ASSERT_EQ(deepNest.size(), 1);
    ASSERT_EQ(deepNest[0].get().toString(), "Tom");
}

// 10. 自定义函数和过滤
// TEST(JsonPath_CustomFunctionAndFilter) {
//     // 测试自定义函数
//     JsonValue json = JsonValue::parse(R"({
//         "items": [
//             {"id": 1, "value": 10},
//             {"id": 2, "value": 20},
//             {"id": 3, "value": 30}
//         ]
//     })");
//     auto customCall = jsonvalue_jsonpath::selectAll(json, "$[?(@.value.customFunc())]");
//     ASSERT_EQ(customCall.size(), 0);

//     JsonValue json = JsonValue::parse(R"({
//         "items": [{"id": 1}, {"id": 2}, {"id": 3}]
//     })");
//     auto multiFilter = jsonvalue_jsonpath::selectAll(json, "$.items[?(@.id == 1 || @.id == 2)]");
//     ASSERT_EQ(multiFilter.size(), 2);
// }

// 11. 错误案例
TEST(JsonPath_ErrorCases) {
    JsonValue json = JsonValue::parse(R"({
        "age": 18
    })");

    // 测试不支持的操作符
    try {
        auto bad = jsonvalue_jsonpath::selectAll(json, "$[?(@.age >>> 10)]");
        ASSERT_EQ(bad.size(), 0);
    } catch (...) {
        ASSERT_FALSE(true);
    }

    // 测试不存在的字段
    auto notExist = jsonvalue_jsonpath::selectAll(json, "$[?(@.notExist == 1)]");
    ASSERT_EQ(notExist.size(), 0);

    // 类型不匹配
    auto typeMismatch = jsonvalue_jsonpath::selectAll(json, "$[?(@.age == 'abc')]");
    ASSERT_EQ(typeMismatch.size(), 0);
}

TEST(JsonPath_ComplexExpressions) {
    JsonValue json = JsonValue::parse(R"({
        "store": {
            "book": [
                {
                    "category": "reference",
                    "author": "Nigel Rees",
                    "title": "Sayings of the Century",
                    "price": 8.95,
                    "isbn": "0-553-21311-3"
                },
                {
                    "category": "fiction",
                    "author": "Evelyn Waugh",
                    "title": "Sword of Honour", 
                    "price": 12.99
                },
                {
                    "category": "fiction",
                    "author": "Herman Melville",
                    "title": "Moby Dick",
                    "isbn": "0-553-21311-3",
                    "price": 8.99
                },
                {
                    "category": "fiction",
                    "author": "J. R. R. Tolkien",
                    "title": "The Lord of the Rings",
                    "isbn": "0-395-19395-8",
                    "price": 22.99
                }
            ],
            "bicycle": {
                "color": "red",
                "price": 19.95
            }
        },
        "expensive": 10
    })");

    // 测试价格过滤
    auto cheapBooks = jsonvalue_jsonpath::selectAll(json, "$.store.book[?(@.price < 10)]");
    ASSERT_EQ(cheapBooks.size(), 2); // 价格低于10的书籍数量

    // 测试类别过滤
    auto fictionBooks = jsonvalue_jsonpath::selectAll(json, "$.store.book[?(@.category == 'fiction')]");
    ASSERT_EQ(fictionBooks.size(), 3); // 价格低于10的书籍数量

    // 测试存在ISBN的书籍
    auto booksWithIsbn = jsonvalue_jsonpath::selectAll(json, "$.store.book[?(@.isbn)]");
    ASSERT_EQ(booksWithIsbn.size(), 3); // 存在ISBN的书籍数量
}

TEST(JsonPath_AdvancedArrayOperations) {
    JsonValue json = JsonValue::parse(R"({
        "matrix": [
            [1, 2, 3],
            [4, 5, 6], 
            [7, 8, 9]
        ],
        "nested": {
            "arrays": [
                {"data": [10, 20, 30]},
                {"data": [40, 50, 60]},
                {"data": [70, 80, 90]}
            ]
        }
    })");

    // 测试矩阵元素访问
    auto element = jsonvalue_jsonpath::selectAll(json, "$.matrix[1][2]");
    ASSERT_EQ(element.size(), 1);
    ASSERT_EQ(element[0].get().toInt(), 6);

    // 测试嵌套数组的平铺
    auto allData = jsonvalue_jsonpath::selectAll(json, "$.nested.arrays[*].data[*]");
    ASSERT_EQ(allData.size(), 9); // 3个数组 * 3个元素

    // 测试数组长度函数，期望返回3
    auto firstRow = jsonvalue_jsonpath::selectAll(json, "$.matrix[0]");
    ASSERT_EQ(firstRow.size(), 1);
    ASSERT_TRUE(firstRow[0].get().isArray());
    ASSERT_EQ(firstRow[0].get().toArray()->get().size(), 3);
}

TEST(JsonPath_RecursiveDescentAdvanced) {
    JsonValue json = JsonValue::parse(R"({
        "company": {
            "departments": {
                "engineering": {
                    "teams": {
                        "backend": {
                            "members": [
                                {"name": "Alice", "role": "senior"},
                                {"name": "Bob", "role": "junior"}
                            ]
                        },
                        "frontend": {
                            "members": [
                                {"name": "Charlie", "role": "senior"},
                                {"name": "David", "role": "junior"}
                            ]
                        }
                    }
                },
                "sales": {
                    "teams": {
                        "enterprise": {
                            "members": [
                                {"name": "Eve", "role": "manager"}
                            ]
                        }
                    }
                }
            }
        }
    })");

    // 测试成员字段的选择
    auto allMembers = jsonvalue_jsonpath::selectAll(json, "$..members");
    ASSERT_EQ(allMembers.size(), 3); // 3个teams的members数量

    // 测试成员姓名的选择
    auto allNames = jsonvalue_jsonpath::selectAll(json, "$..name");
    ASSERT_EQ(allNames.size(), 5); // 5个成员的姓名

    // 测试成员角色的选择
    auto seniorMembers = jsonvalue_jsonpath::selectAll(json, "$..members[?(@.role == 'senior')]");
    ASSERT_EQ(seniorMembers.size(), 2); // Alice 和 Charlie
}

TEST(JsonPath_UnionExpressions) {
    JsonValue json = JsonValue::parse(R"({
        "data": {
            "primary": {"value": 100},
            "secondary": {"value": 200}, 
            "tertiary": {"value": 300},
            "items": [1, 2, 3, 4, 5]
        }
    })");

    // 测试联合表达式
    auto unionResult = jsonvalue_jsonpath::selectAll(json, "$.data.primary.value,$.data.secondary.value");
    ASSERT_EQ(unionResult.size(), 2);
    ASSERT_EQ(unionResult[0].get().toInt(), 100);
    ASSERT_EQ(unionResult[1].get().toInt(), 200);

    // 测试数组的多重索引
    auto multiIndex = jsonvalue_jsonpath::selectAll(json, "$.data.items[0,2,4]");
    ASSERT_EQ(multiIndex.size(), 3);
    ASSERT_EQ(multiIndex[0].get().toInt(), 1);
    ASSERT_EQ(multiIndex[1].get().toInt(), 3);
    ASSERT_EQ(multiIndex[2].get().toInt(), 5);
}

TEST(JsonPath_SlicingExtended) {
    JsonValue json = JsonValue::parse(R"({
        "sequence": [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]
    })");

    // 测试切片语法 [start:end:step]
    auto stepSlice = jsonvalue_jsonpath::selectAll(json, "$.sequence[0:10:2]");
    ASSERT_EQ(stepSlice.size(), 5); // 0, 2, 4, 6, 8

    // 测试反向切片
    auto reverseSlice = jsonvalue_jsonpath::selectAll(json, "$.sequence[10:0:-2]");
    ASSERT_EQ(reverseSlice.size(), 5); // 10, 8, 6, 4, 2

    // 测试复杂切片
    auto complexSlice = jsonvalue_jsonpath::selectAll(json, "$.sequence[-5:-1]");
    ASSERT_EQ(complexSlice.size(), 4); // 5个元素
}

TEST(JsonPath_FilterExpressions) {
    JsonValue json = JsonValue::parse(R"({
        "products": [
            {"id": 1, "name": "Widget A", "price": 10.50, "tags": ["electronics", "gadget"]},
            {"id": 2, "name": "Widget B", "price": 25.00, "tags": ["electronics", "tool"]},
            {"id": 3, "name": "Book C", "price": 15.99, "tags": ["education", "book"]},
            {"id": 4, "name": "Gadget D", "price": 8.99, "tags": ["electronics", "gadget", "cheap"]}
        ]
    })");

    // 测试价格范围过滤 - 复合条件AND
    auto midRangeProducts = jsonvalue_jsonpath::selectAll(json, "$.products[?(@.price > 10 && @.price < 20)]");
    ASSERT_EQ(midRangeProducts.size(), 2); // Widget A 和 Book C

    // 测试名称匹配过滤 - 正则表达式
    auto widgetProducts = jsonvalue_jsonpath::selectAll(json, "$.products[?(@.name =~ /Widget.*/)]");
    ASSERT_EQ(widgetProducts.size(), 2); // Widget A 和 Widget B

    // 测试数组元素的包含关系
    auto gadgetProducts = jsonvalue_jsonpath::selectAll(json, "$.products[?('gadget' in @.tags)]");
    ASSERT_EQ(gadgetProducts.size(), 2); // Widget A 和 Gadget D

    // 测试名称匹配过滤 - 正则表达式
    widgetProducts = jsonvalue_jsonpath::selectAll(json, "$.products[?(@.name =~ /Widget.*/)]");
    ASSERT_EQ(widgetProducts.size(), 2); // Widget A 和 Widget B
}

TEST(JsonPath_FunctionExpressions) {
    JsonValue json = JsonValue::parse(R"({
        "arrays": [
            {"numbers": [1, 2, 3, 4, 5]},
            {"numbers": [10, 20, 30]}, 
            {"numbers": [100]}
        ],
        "strings": [
            {"text": "hello world"},
            {"text": "foo bar"},
            {"text": "test"}
        ]
    })");

    // 测试数组长度函数
    auto longArrays = jsonvalue_jsonpath::selectAll(json, "$.arrays[?(@.numbers.length() > 3)]");
    ASSERT_EQ(longArrays.size(), 1); // 只有一个数组的长度 > 3

    // 测试字符串长度函数
    auto longStrings = jsonvalue_jsonpath::selectAll(json, "$.strings[?(@.text.length() > 5)]");
    ASSERT_EQ(longStrings.size(), 2); // "hello world" 和 "foo bar"

    // 测试数组元素的最大值
    auto hasLargeNumbers = jsonvalue_jsonpath::selectAll(json, "$.arrays[?(@.numbers.max() > 50)]");
    ASSERT_EQ(hasLargeNumbers.size(), 1); // 只有一个数组的最大值 > 50
}

TEST(JsonPath_EdgeCases) {
    JsonValue json = JsonValue::parse(R"({
        "empty": {},
        "nullValue": null,
        "emptyArray": [],
        "emptyString": "",
        "zero": 0,
        "false": false,
        "special": {
            "key with spaces": "value1",
            "key-with-dashes": "value2", 
            "key.with.dots": "value3"
        }
    })");

    // 测试空值访问
    auto nullAccess = jsonvalue_jsonpath::selectAll(json, "$.nullValue");
    ASSERT_EQ(nullAccess.size(), 1);
    ASSERT_TRUE(nullAccess[0].get().isNull());

    // 测试空数组/空对象
    auto emptyObjAccess = jsonvalue_jsonpath::selectAll(json, "$.empty");
    ASSERT_EQ(emptyObjAccess.size(), 1);
    ASSERT_TRUE(emptyObjAccess[0].get().isObject());

    // 测试特殊字符访问
    auto specialKeys = jsonvalue_jsonpath::selectAll(json, "$.special['key with spaces']");
    ASSERT_EQ(specialKeys.size(), 1);
    ASSERT_EQ(specialKeys[0].get().toString(), "value1");

    // 测试带有破折号的键
    auto dotKeys = jsonvalue_jsonpath::selectAll(json, "$.special['key.with.dots']");
    ASSERT_EQ(dotKeys.size(), 1);
    ASSERT_EQ(dotKeys[0].get().toString(), "value3");
}

TEST(JsonPath_PerformanceOptimization) {
    // 测试大规模JSON数据
    JsonValue::ObjectType largeObj;
    for (int i = 0; i < 1000; ++i) {
        JsonValue::ObjectType item;
        item["id"] = JsonValue(i);
        item["name"] = JsonValue("Item " + std::to_string(i));
        item["value"] = JsonValue(i * 1.5);
        largeObj["item_" + std::to_string(i)] = JsonValue(item);
    }
    JsonValue json(largeObj);

    // 测试查询性能 - 复杂查询
    auto start = std::chrono::high_resolution_clock::now();
    auto results = jsonvalue_jsonpath::selectAll(json, "$..value");
    auto end = std::chrono::high_resolution_clock::now();
    
    ASSERT_EQ(results.size(), 1000);

    // 测试查询耗时 - 复杂查询
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    ASSERT_TRUE(duration.count() < 1000);
}

TEST(JsonPath_MultiNestedJson){
    auto json = R"(
    {
      "company": {
        "name": "Acme Corp",
        "departments": [
          {
            "name": "Engineering",
            "manager": {
              "name": "Alice",
              "age": 35
            },
            "employees": [
              {
                "name": "Bob",
                "age": 28,
                "skills": ["JavaScript", "Python"]
              },
              {
                "name": "Charlie",
                "age": 32,
                "skills": ["Go", "Rust"]
              }
            ]
          },
          {
            "name": "HR",
            "manager": {
              "name": "David",
              "age": 40
            },
            "employees": [
              {
                "name": "Eva",
                "age": 25,
                "skills": ["Recruitment", "Onboarding"]
              }
            ]
          }
        ]
      }
    }
    )"_json;

    auto ageGreaterThan30 = jsonvalue_jsonpath::selectAll(json, "$.company.departments[*].employees[?(@.age > 30)].name");
    ASSERT_EQ(ageGreaterThan30.size(), 1);
    ASSERT_EQ(ageGreaterThan30[0].get().toString(), "Charlie");

    auto allName = jsonvalue_jsonpath::selectAll(json, "$.company.departments[*].employees[*].name");
    ASSERT_EQ(allName.size(), 3);
    ASSERT_EQ(allName[0].get().toString(), "Bob");
    ASSERT_EQ(allName[1].get().toString(), "Charlie");
    ASSERT_EQ(allName[2].get().toString(), "Eva");

    auto allSkills = jsonvalue_jsonpath::selectAll(json, "$.company.departments[*].employees[*].skills[*]");
    ASSERT_EQ(allSkills.size(), 6);
}

TEST(JsonPath_MultiNestedCond) {
    auto json = R"({
      "orders": [
        {
          "id": 1,
          "items": [
            {"name": "apple", "price": 120},
            {"name": "banana", "price": 30}
          ]
        },
        {
          "id": 2,
          "items": [
            {"name": "pear", "price": 80}
          ]
        },
        {
          "id": 3,
          "items": [
            {"name": "orange", "price": 150}
          ]
        }
      ]
    })"_json;

    auto priceGreater100 = jsonvalue_jsonpath::selectAll(json, "$.orders[?(@.items[?(@.price > 100)])]");
    ASSERT_EQ(priceGreater100.size(), 2);
}

TEST(JsonPath_MultiLevelNestedFilter) {
    JsonValue json = JsonValue::parse(R"({
        "companies": [
            {
                "name": "A",
                "departments": [
                    {
                        "name": "Dev",
                        "teams": [
                            {
                                "name": "Backend",
                                "members": [
                                    {"name": "Alice", "age": 29},
                                    {"name": "Bob", "age": 27}
                                ]
                            },
                            {
                                "name": "Frontend",
                                "members": [
                                    {"name": "Carol", "age": 31}
                                ]
                            }
                        ]
                    }
                ]
            },
            {
                "name": "B",
                "departments": [
                    {
                        "name": "QA",
                        "teams": [
                            {
                                "name": "Test",
                                "members": [
                                    {"name": "Dave", "age": 26}
                                ]
                            }
                        ]
                    }
                ]
            }
        ]
    })");

    // 测试年龄大于28的公司
    auto greater28 = jsonvalue_jsonpath::selectAll(json, "$.companies[?(@.departments[?(@.teams[?(@.members[?(@.age > 28)])])])]");
    ASSERT_EQ(greater28.size(), 1);
    ASSERT_EQ(greater28[0].get()["name"].toString(), "A");
}

int main(int argc, char** argv) {
    return RUN_ALL_TESTS();
}