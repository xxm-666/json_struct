#include <iostream>
#include <cassert>
#include <string>
#include "../src/json_engine/json_path.h"
#include "../src/json_engine/json_value.h"

using namespace JsonStruct;
using namespace jsonvalue_jsonpath;

void test_single_value_modification() {
    std::cout << "Testing single value modification..." << std::endl;
    
    // Create test JSON
    JsonValue root;
    root["store"]["book"][0]["title"] = JsonValue("Book1");
    root["store"]["book"][0]["price"] = JsonValue(10.0);
    root["store"]["book"][1]["title"] = JsonValue("Book2");
    root["store"]["book"][1]["price"] = JsonValue(15.0);
    
    // Test selecting and modifying first book's price
    auto first_price = selectFirstMutable(root, "$.store.book[0].price");
    assert(first_price.has_value());
    
    // Modify the price
    first_price->get() = JsonValue(12.0);
    
    // Verify modification
    auto verify_price = selectFirst(root, "$.store.book[0].price");
    assert(verify_price.has_value());
    assert(verify_price->get().toDouble() == 12.0);
    
    std::cout << "Single value modification test passed!" << std::endl;
}

void test_batch_modification() {
    std::cout << "Testing batch modification..." << std::endl;
    
    // Create test JSON
    JsonValue root;
    root["products"][0]["price"] = JsonValue(100.0);
    root["products"][0]["discount"] = JsonValue(false);
    root["products"][1]["price"] = JsonValue(200.0);
    root["products"][1]["discount"] = JsonValue(false);
    root["products"][2]["price"] = JsonValue(300.0);
    root["products"][2]["discount"] = JsonValue(false);
    
    // Test selecting all prices
    auto all_prices = selectAllMutable(root, "$.products[*].price");
    assert(all_prices.size() == 3);
    
    // Apply discount to all products (10% off)
    for (auto& price_ref : all_prices) {
        auto& price = price_ref.get();
        double current_price = price.toDouble();
        price = JsonValue(current_price * 0.9); // 10% discount
    }
    
    // Verify all prices were modified
    auto verify_prices = selectAll(root, "$.products[*].price");
    assert(verify_prices.size() == 3);
    assert(verify_prices[0].get().toDouble() == 90.0);
    assert(verify_prices[1].get().toDouble() == 180.0);
    assert(verify_prices[2].get().toDouble() == 270.0);
    
    std::cout << "Batch modification test passed!" << std::endl;
}

void test_nested_modification() {
    std::cout << "Testing nested modification..." << std::endl;
    
    // Create test JSON
    JsonValue root;
    root["data"]["items"][0]["info"]["status"] = JsonValue("pending");
    root["data"]["items"][1]["info"]["status"] = JsonValue("pending");
    root["data"]["items"][2]["info"]["status"] = JsonValue("pending");
    
    // Test recursive descent to find all status fields
    auto all_status = selectAllMutable(root, "$..status");
    assert(all_status.size() == 3);
    
    // Update all status to "completed"
    for (auto& status_ref : all_status) {
        status_ref.get() = JsonValue("completed");
    }
    
    // Verify all status were updated
    auto verify_status = selectAll(root, "$..status");
    assert(verify_status.size() == 3);
    for (const auto& status : verify_status) {
        assert(status.get().toString() == "completed");
    }
    
    std::cout << "Nested modification test passed!" << std::endl;
}

void test_array_modification() {
    std::cout << "Testing array modification..." << std::endl;
    
    // Create test JSON
    JsonValue root;
    root["numbers"][0] = JsonValue(1);
    root["numbers"][1] = JsonValue(2);
    root["numbers"][2] = JsonValue(3);
    root["numbers"][3] = JsonValue(4);
    root["numbers"][4] = JsonValue(5);
    
    // Test modifying array slice
    auto slice = selectAllMutable(root, "$.numbers[1:4]");
    assert(slice.size() == 3);
    
    // Multiply each value by 10
    for (auto& num_ref : slice) {
        auto& num = num_ref.get();
        num = JsonValue(num.toInt() * 10);
    }
    
    // Verify modifications
    auto verify_all = selectAll(root, "$.numbers[*]");
    assert(verify_all.size() == 5);
    assert(verify_all[0].get().toInt() == 1);   // unchanged
    assert(verify_all[1].get().toInt() == 20);  // modified
    assert(verify_all[2].get().toInt() == 30);  // modified
    assert(verify_all[3].get().toInt() == 40);  // modified
    assert(verify_all[4].get().toInt() == 5);   // unchanged
    
    std::cout << "Array modification test passed!" << std::endl;
}

void test_conditional_modification() {
    std::cout << "Testing conditional modification..." << std::endl;
    
    // Create test JSON
    JsonValue root;
    root["inventory"][0]["item"] = JsonValue("apple");
    root["inventory"][0]["quantity"] = JsonValue(5);
    root["inventory"][1]["item"] = JsonValue("banana");
    root["inventory"][1]["quantity"] = JsonValue(0);
    root["inventory"][2]["item"] = JsonValue("orange");
    root["inventory"][2]["quantity"] = JsonValue(3);
    
    // Find all items and check their quantities
    auto all_items = selectAllMutable(root, "$.inventory[*]");
    
    // Update quantity to -1 for out-of-stock items (quantity == 0)
    for (auto& item_ref : all_items) {
        auto& item = item_ref.get();
        if (item["quantity"].toInt() == 0) {
            item["quantity"] = JsonValue(-1);
        }
    }
    
    // Verify modification
    auto banana_qty = selectFirst(root, "$.inventory[1].quantity");
    assert(banana_qty.has_value());
    assert(banana_qty->get().toInt() == -1);
    
    // Verify others unchanged
    auto apple_qty = selectFirst(root, "$.inventory[0].quantity");
    auto orange_qty = selectFirst(root, "$.inventory[2].quantity");
    assert(apple_qty->get().toInt() == 5);
    assert(orange_qty->get().toInt() == 3);

	auto hasApple = selectAll(root, "$.inventory[?(@.item == 'apple')]");
	assert(hasApple.size() == 1);

	auto greaterThanZero = selectAll(root, "$.inventory[?(@.quantity > 0)]");
	assert(greaterThanZero.size() == 2); // apple and orange should remain
    
    std::cout << "Conditional modification test passed!" << std::endl;
}

void test_query_result_modification() {
    std::cout << "Testing MutableQueryResult usage..." << std::endl;
    
    // Create test JSON
    JsonValue root;
    root["config"]["timeout"] = JsonValue(30);
    root["config"]["retries"] = JsonValue(3);
    root["config"]["debug"] = JsonValue(false);
    
    // Use queryMutable to get all config values
    auto result = queryMutable(root, "$.config.*");
    assert(result.size() == 3);
    
    // Modify values through the result
    for (auto& value_ref : result.values) {
        auto& value = value_ref.get();
        if (value.isNumber() && value.isInteger()) {
            value = JsonValue(value.toInt() * 2);
        } else if (value.isBool()) {
            value = JsonValue(!value.toBool());
        }
    }
    
    // Verify modifications
    assert(root["config"]["timeout"].toInt() == 60);
    assert(root["config"]["retries"].toInt() == 6);
    assert(root["config"]["debug"].toBool() == true);
    
    std::cout << "MutableQueryResult test passed!" << std::endl;
}

int main() {
    try {
        test_single_value_modification();
        test_batch_modification();
        test_nested_modification();
        test_array_modification();
        test_conditional_modification();
        test_query_result_modification();
        
        std::cout << "\nAll tests passed! JSONPath mutable operations work correctly." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
