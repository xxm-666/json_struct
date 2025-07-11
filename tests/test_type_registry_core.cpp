#include "jsonstruct.h"
#include <cassert>
#include <iostream>
#include <typeindex>

using namespace JsonStruct;

void test_registry_core() {
    TypeRegistry& reg = TypeRegistry::instance();
    reg.clear();
    assert(!reg.isRegistered<int>());
    reg.registerType<int>(
        [](const int& v){ return JsonValue(v); },
        [](const JsonValue& j, const int& def){ return j.isNull() ? def : j.toInt(def); }
    );
    assert(reg.isRegistered<int>());
    JsonValue j = reg.toJson(123);
    assert(j.isNumber() && j.toInt() == 123);
    int x = reg.fromJson<int>(j, 0);
    assert(x == 123);
    auto types = reg.getRegisteredTypes();
    assert(!types.empty());
    reg.clear();
    assert(reg.getRegisteredTypes().empty());
}

struct SimpleStruct { 
    int x; 
    double y; 
    std::string z; 
    JSON_FIELDS(x, y, z)
};

void test_field_macros() {
    auto names = SimpleStruct::get_field_names();
    assert(names.size() == 3);
    assert(names[0] == "x");
    assert(names[1] == "y");
    assert(names[2] == "z");
    SimpleStruct a{1, 2.5, "hi"};
    auto tup = a.json_fields();
    assert(std::get<0>(tup) == 1);
    assert(std::get<1>(tup) == 2.5);
    assert(std::get<2>(tup) == "hi");
}

int main() {
    test_registry_core();
    test_field_macros();
    std::cout << "Basic TypeRegistry tests passed." << std::endl;
    return 0;
}
