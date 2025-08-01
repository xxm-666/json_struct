#include "../test_framework/test_framework.h"
#include "../src/type_registry/registry_core.h"
#include "../src/type_registry/auto_serializer.h"
#include <string>
#include <vector>

using namespace JsonStruct;

// 复杂的嵌套结构测试
struct Address {
    JSON_AUTO(street, city, zipCode)
    std::string street;
    std::string city;
    int zipCode = 0;
};

struct PhoneNumber {
    JSON_AUTO(type, number)
    std::string type;
    std::string number;
};

struct Person {
    JSON_AUTO(name, age, email, address, phoneNumbers)
    std::string name;
    int age = 0;
    std::string email;
    Address address;
    std::vector<PhoneNumber> phoneNumbers;
};

struct Company {
    JSON_AUTO(name, employees, headquarters)
    std::string name;
    std::vector<Person> employees;
    Address headquarters;
};

TEST(ComplexStructures_AddressSerialization) {
    Address addr;
    addr.street = "123 Main St";
    addr.city = "Anytown";
    addr.zipCode = 12345;
    
    JsonValue json = JsonValue(toJson(addr));
    ASSERT_TRUE(json.isObject());
    
    Address addr2;
    fromJson(addr2, json);
    
    ASSERT_EQ(addr.street, addr2.street);
    ASSERT_EQ(addr.city, addr2.city);
    ASSERT_EQ(addr.zipCode, addr2.zipCode);
}

TEST(ComplexStructures_PersonWithMultiplePhones) {
    Person person;
    person.name = "John Doe";
    person.age = 30;
    person.email = "john@example.com";
    person.address.street = "456 Oak Ave";
    person.address.city = "Springfield";
    person.address.zipCode = 67890;
    
    PhoneNumber phone1 = {"home", "555-0123"};
    PhoneNumber phone2 = {"work", "555-0456"};
    person.phoneNumbers = {phone1, phone2};
    
    JsonValue json = JsonValue(toJson(person));
    ASSERT_TRUE(json.isObject());
    
    Person person2;
    fromJson(person2, json);
    
    ASSERT_EQ(person.name, person2.name);
    ASSERT_EQ(person.age, person2.age);
    ASSERT_EQ(person.email, person2.email);
    ASSERT_EQ(person.address.street, person2.address.street);
    ASSERT_EQ(person.phoneNumbers.size(), person2.phoneNumbers.size());
    if (!person.phoneNumbers.empty()) {
        ASSERT_EQ(person.phoneNumbers[0].type, person2.phoneNumbers[0].type);
        ASSERT_EQ(person.phoneNumbers[0].number, person2.phoneNumbers[0].number);
    }
}

TEST(ComplexStructures_CompanyWithEmployees) {
    Company company;
    company.name = "Tech Corp";
    company.headquarters.street = "100 Business Blvd";
    company.headquarters.city = "Tech City";
    company.headquarters.zipCode = 55555;
    
    Person emp1;
    emp1.name = "Alice Smith";
    emp1.age = 28;
    emp1.email = "alice@techcorp.com";
    emp1.address.street = "789 Tech St";
    emp1.address.city = "Tech City";
    emp1.address.zipCode = 55556;
    emp1.phoneNumbers = {{"work", "555-1000"}};
    
    Person emp2;
    emp2.name = "Bob Johnson";
    emp2.age = 35;
    emp2.email = "bob@techcorp.com";
    emp2.address.street = "321 Code Ave";
    emp2.address.city = "Tech City";
    emp2.address.zipCode = 55557;
    emp2.phoneNumbers = {{"work", "555-2000"}, {"mobile", "555-2001"}};
    
    company.employees = {emp1, emp2};
    
    JsonValue json = JsonValue(toJson(company));
    ASSERT_TRUE(json.isObject());
    
    Company company2;
    fromJson(company2, json);
    
    ASSERT_EQ(company.name, company2.name);
    ASSERT_EQ(company.headquarters.city, company2.headquarters.city);
    ASSERT_EQ(company.employees.size(), company2.employees.size());
    if (!company.employees.empty()) {
        ASSERT_EQ(company.employees[0].name, company2.employees[0].name);
        if (company.employees.size() > 1) {
            ASSERT_EQ(company.employees[1].phoneNumbers.size(), company2.employees[1].phoneNumbers.size());
        }
    }
}

int main() {
    return RUN_ALL_TESTS();
}
