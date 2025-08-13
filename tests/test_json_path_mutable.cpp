#include <iostream>
#include <cassert>
#include <string>
#include "../src/json_engine/json_path.h"
#include "../src/json_engine/json_value.h"
#include <test_framework/test_framework.h>

using namespace JsonStruct;
using namespace jsonvalue_jsonpath;
using namespace TestFramework;

TEST(SingleValueModification) {
    JsonValue root;
    root["store"]["book"][0]["title"] = JsonValue("Book1");
    root["store"]["book"][0]["price"] = JsonValue(10.0);
    root["store"]["book"][1]["title"] = JsonValue("Book2");
    root["store"]["book"][1]["price"] = JsonValue(15.0);
    auto first_price = selectFirstMutable(root, "$.store.book[0].price");
    ASSERT_TRUE(first_price.has_value());
    first_price->get() = JsonValue(12.0);
    auto verify_price = selectFirst(root, "$.store.book[0].price");
    ASSERT_TRUE(verify_price.has_value());
    ASSERT_TRUE(verify_price->get().toDouble() == 12.0);
}

TEST(BatchModification) {
    JsonValue root;
    root["products"][0]["price"] = JsonValue(100.0);
    root["products"][0]["discount"] = JsonValue(false);
    root["products"][1]["price"] = JsonValue(200.0);
    root["products"][1]["discount"] = JsonValue(false);
    root["products"][2]["price"] = JsonValue(300.0);
    root["products"][2]["discount"] = JsonValue(false);
    auto all_prices = selectAllMutable(root, "$.products[*].price");
    ASSERT_TRUE(all_prices.size() == 3);
    for (auto& price_ref : all_prices) {
        auto& price = price_ref.get();
        double current_price = price.toDouble();
        price = JsonValue(current_price * 0.9);
    }
    auto verify_prices = selectAll(root, "$.products[*].price");
    ASSERT_TRUE(verify_prices.size() == 3);
    ASSERT_TRUE(verify_prices[0].get().toDouble() == 90.0);
    ASSERT_TRUE(verify_prices[1].get().toDouble() == 180.0);
    ASSERT_TRUE(verify_prices[2].get().toDouble() == 270.0);
}

TEST(NestedModification) {
    JsonValue root;
    root["data"]["items"][0]["info"]["status"] = JsonValue("pending");
    root["data"]["items"][1]["info"]["status"] = JsonValue("pending");
    root["data"]["items"][2]["info"]["status"] = JsonValue("pending");
    auto all_status = selectAllMutable(root, "$..status");
    ASSERT_TRUE(all_status.size() == 3);
    for (auto& status_ref : all_status) {
        status_ref.get() = JsonValue("completed");
    }
    auto verify_status = selectAll(root, "$..status");
    ASSERT_TRUE(verify_status.size() == 3);
    for (const auto& status : verify_status) {
        ASSERT_TRUE(status.get().toString() == "completed");
    }
}

TEST(ArrayModification) {
    JsonValue root;
    root["numbers"][0] = JsonValue(1);
    root["numbers"][1] = JsonValue(2);
    root["numbers"][2] = JsonValue(3);
    root["numbers"][3] = JsonValue(4);
    root["numbers"][4] = JsonValue(5);
    auto slice = selectAllMutable(root, "$.numbers[1:4]");
    ASSERT_TRUE(slice.size() == 3);
    for (auto& num_ref : slice) {
        auto& num = num_ref.get();
        num = JsonValue(num.toInt() * 10);
    }
    auto verify_all = selectAll(root, "$.numbers[*]");
    ASSERT_TRUE(verify_all.size() == 5);
    ASSERT_TRUE(verify_all[0].get().toInt() == 1);
    ASSERT_TRUE(verify_all[1].get().toInt() == 20);
    ASSERT_TRUE(verify_all[2].get().toInt() == 30);
    ASSERT_TRUE(verify_all[3].get().toInt() == 40);
    ASSERT_TRUE(verify_all[4].get().toInt() == 5);
}

TEST(ConditionalModification) {
    JsonValue root;
    root["inventory"][0]["item"] = JsonValue("apple");
    root["inventory"][0]["quantity"] = JsonValue(5);
    root["inventory"][1]["item"] = JsonValue("banana");
    root["inventory"][1]["quantity"] = JsonValue(0);
    root["inventory"][2]["item"] = JsonValue("orange");
    root["inventory"][2]["quantity"] = JsonValue(3);
    auto all_items = selectAllMutable(root, "$.inventory[*]");
    for (auto& item_ref : all_items) {
        auto& item = item_ref.get();
        if (item["quantity"].toInt() == 0) {
            item["quantity"] = JsonValue(-1);
        }
    }
    auto banana_qty = selectFirst(root, "$.inventory[1].quantity");
    ASSERT_TRUE(banana_qty.has_value());
    ASSERT_TRUE(banana_qty->get().toInt() == -1);
    auto apple_qty = selectFirst(root, "$.inventory[0].quantity");
    auto orange_qty = selectFirst(root, "$.inventory[2].quantity");
    ASSERT_TRUE(apple_qty->get().toInt() == 5);
    ASSERT_TRUE(orange_qty->get().toInt() == 3);
    auto hasApple = selectAll(root, "$.inventory[?(@.item == 'apple')]");
    ASSERT_TRUE(hasApple.size() == 1);
    auto greaterThanZero = selectAll(root, "$.inventory[?(@.quantity > 0)]");
    ASSERT_TRUE(greaterThanZero.size() == 2);
}

TEST(QueryResultModification) {
    JsonValue root;
    root["config"]["timeout"] = JsonValue(30);
    root["config"]["retries"] = JsonValue(3);
    root["config"]["debug"] = JsonValue(false);
    auto result = queryMutable(root, "$.config.*");
    ASSERT_TRUE(result.size() == 3);
    for (auto& value_ref : result.values) {
        auto& value = value_ref.get();
        if (value.isNumber() && value.isInteger()) {
            value = JsonValue(value.toInt() * 2);
        } else if (value.isBool()) {
            value = JsonValue(!value.toBool());
        }
    }
    ASSERT_TRUE(root["config"]["timeout"].toInt() == 60);
    ASSERT_TRUE(root["config"]["retries"].toInt() == 6);
    ASSERT_TRUE(root["config"]["debug"].toBool() == true);
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}
