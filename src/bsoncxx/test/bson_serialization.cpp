// Copyright 2020 MongoDB Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/string/to_string.hpp>
#include <bsoncxx/test_util/catch.hh>
#include <bsoncxx/types/bson_value/view.hpp>

using namespace bsoncxx;

using builder::basic::kvp;
using builder::basic::make_array;
using builder::basic::make_document;

namespace {

// Forward declaration for helper functions
namespace test {
class Car;
class Person;
}

// Helper function to that creates an array for the basic builder
builder::basic::array create_array(const int arr[], int size);
builder::basic::array create_array(const test::Person people[], int size);

namespace test {

class Car {
   public:
    Car() = default;
    Car(std::string manufacturer, std::string model)
        : _manufacturer(std::move(manufacturer)), _model(std::move(model)){};

    std::string get_manufacturer() const {
        return _manufacturer;
    }

    std::string get_model() const {
        return _model;
    }

    friend void to_bson(const Car& car, bsoncxx::document::value& bson_object) {
        auto builder = bsoncxx::builder::stream::document{};
        bson_object = builder << "manufacturer" << car._manufacturer << "model" << car._model
                              << bsoncxx::builder::stream::finalize;
    }

    friend void from_bson(Car& car, const bsoncxx::document::view& bson_object) {
        bson_object["manufacturer"].get_value().to_field(car._manufacturer);
        bson_object["model"].get_value().to_field(car._model);
    }

   private:
    std::string _manufacturer;
    std::string _model;
};

struct Person {
    std::string first_name;
    std::string last_name;
    int age;
    Car* car;
    static const int fave_numbers_size = 4;
    int fave_numbers[fave_numbers_size];
};

// to_bson serializer function that can take a document::value&
void to_bson(const Person& person, bsoncxx::document::value& bson_object) {
    bson_object = make_document(
        kvp("first_name", person.first_name),
        kvp("last_name", person.last_name),
        kvp("age", person.age),
        kvp("car", document::value(*(person.car))),
        kvp("fave_numbers", create_array(person.fave_numbers, test::Person::fave_numbers_size)));
}

void from_bson(Person& person, const bsoncxx::document::view& bson_object) {
    bson_object["first_name"].get_value().to_field(person.first_name);
    bson_object["last_name"].get_value().to_field(person.last_name);
    bson_object["age"].get_value().to_field(person.age);

    person.car = new Car;
    bson_object["car"].get_value().to_field(*(person.car));

    bson_object["fave_numbers"].get_value().to_field(person.fave_numbers);
}

// Class that uses most BSON Types
class Types_Test_Object {
   public:
    Types_Test_Object() = default;

    Types_Test_Object(double m_double,
                      std::string&& m_utf8,
                      bool m_bool,
                      std::string&& m_code,
                      std::string&& m_symbol,
                      int32_t m_int32,
                      int64_t m_int64,
                      decimal128 m_decimal128,
                      std::string&& m_regex_string,
                      std::string&& m_regex_options,
                      uint32_t m_timestamp_increment,
                      uint32_t m_timestamp_timestamp,
                      Person& m_person)
        : m_double(m_double),
          m_utf8(std::move(m_utf8)),
          m_bool(m_bool),
          m_code(std::move(m_code)),
          m_symbol(std::move(m_symbol)),
          m_int32(m_int32),
          m_int64(m_int64),
          m_decimal128(m_decimal128),
          m_regex_string(std::move(m_regex_string)),
          m_regex_options(std::move(m_regex_options)),
          m_timestamp_increment(m_timestamp_increment),
          m_timestamp_timestamp(m_timestamp_timestamp),
          m_person(m_person) {}

    void initialize_people_array(const Person people[]) {
        for (int i = 0; i < m_people_size; i++) {
            m_people[i] = people[i];
        }
    }

    // Getters
    double getDouble() const {
        return m_double;
    }
    std::string getUtf8() const {
        return m_utf8;
    }
    bool getBool() const {
        return m_bool;
    }
    std::string getCode() const {
        return m_code;
    }
    std::string getSymbol() const {
        return m_symbol;
    }
    int32_t getInt32() const {
        return m_int32;
    }
    int64_t getInt64() const {
        return m_int64;
    }
    decimal128 getDecimal128() const {
        return m_decimal128;
    }
    std::string getRegexString() const {
        return m_regex_string;
    }
    std::string getRegexOptions() const {
        return m_regex_options;
    }
    uint32_t getTimestampIncrement() const {
        return m_timestamp_increment;
    }
    uint32_t getTimestampTimestamp() const {
        return m_timestamp_timestamp;
    }
    Person getPerson() const {
        return m_person;
    }
    Person getPersonInArray(int index) {
        return m_people[index];
    }

    friend void to_bson(const Types_Test_Object& user_object,
                        bsoncxx::document::value& bson_object) {
        bson_object = make_document(
            kvp("_double", types::b_double{user_object.m_double}),
            kvp("_utf8", types::b_utf8{user_object.m_utf8}),
            kvp("_bool", types::b_bool{user_object.m_bool}),
            kvp("_code", types::b_code{user_object.m_code}),
            kvp("_symbol", types::b_symbol{user_object.m_symbol}),
            kvp("_int32", types::b_int32{user_object.m_int32}),
            kvp("_int64", types::b_int64{user_object.m_int64}),
            kvp("_decimal128", types::b_decimal128{user_object.m_decimal128}),
            kvp("_regex", types::b_regex{user_object.m_regex_string, user_object.m_regex_options}),
            kvp("_timestamp",
                types::b_timestamp{user_object.m_timestamp_increment,
                                   user_object.m_timestamp_timestamp}),
            kvp("_person", document::value(user_object.m_person)),
            kvp("_people",
                create_array(user_object.m_people, test::Types_Test_Object::m_people_size)));
    }

    friend void from_bson(Types_Test_Object& user_object,
                          const bsoncxx::document::view& bson_object) {
        bson_object["_double"].get_value().to_field(user_object.m_double);
        bson_object["_utf8"].get_value().to_field(user_object.m_utf8);
        bson_object["_bool"].get_value().to_field(user_object.m_bool);
        bson_object["_code"].get_value().to_field(user_object.m_code);
        bson_object["_symbol"].get_value().to_field(user_object.m_symbol);
        bson_object["_int32"].get_value().to_field(user_object.m_int32);
        bson_object["_int64"].get_value().to_field(user_object.m_int64);
        bson_object["_decimal128"].get_value().to_field(user_object.m_decimal128);
        bson_object["_regex"].get_value().to_fields(user_object.m_regex_string,
                                                    user_object.m_regex_options);
        bson_object["_timestamp"].get_value().to_fields(user_object.m_timestamp_increment,
                                                        user_object.m_timestamp_timestamp);
        bson_object["_person"].get_value().to_field(user_object.m_person);
        bson_object["_people"].get_value().to_field(user_object.m_people);
    }

   private:
    // Represent BSON types that only have one struct field
    double m_double;
    std::string m_utf8;
    bool m_bool = true;
    std::string m_code;
    std::string m_symbol;
    int32_t m_int32;
    int64_t m_int64;
    decimal128 m_decimal128;

    // Represent BSON types with more than one struct field
    // Regex
    std::string m_regex_string;
    std::string m_regex_options;
    // Timestamp
    uint32_t m_timestamp_increment = 0;
    uint32_t m_timestamp_timestamp = 0;

    // Nested object
    Person m_person;

    // Array
    static const int m_people_size = 3;
    Person m_people[m_people_size];
};

}  // namespace test

// Helper functions for testing
void compare_car_with_car(const test::Car& c1, const test::Car& c2) {
    REQUIRE(c1.get_model() == c2.get_model());
    REQUIRE(c1.get_manufacturer() == c2.get_manufacturer());
}

void compare_person_with_person(const test::Person& p1, const test::Person& p2) {
    REQUIRE(p1.first_name == p2.first_name);
    REQUIRE(p1.last_name == p2.last_name);
    REQUIRE(p1.age == p2.age);

    for (int i = 0; i < test::Person::fave_numbers_size; i++) {
        REQUIRE(p1.fave_numbers[i] == p2.fave_numbers[i]);
    }

    compare_car_with_car(*(p1.car), *(p2.car));
}

builder::basic::array create_array(const int arr[], int size) {
    builder::basic::array temp;
    for (int i = 0; i < size; i++) {
        temp.append(arr[i]);
    }
    return temp;
}

builder::basic::array create_array(const test::Person people[], int size) {
    builder::basic::array temp;
    for (int i = 0; i < size; i++) {
        temp.append(document::value(people[i]));
    }
    return temp;
}

TEST_CASE("Convert between Car class and BSON object") {
    test::Car expected_car{"Tesla", "Model S"};
    bsoncxx::document::value expected_value =
        make_document(kvp("manufacturer", expected_car.get_manufacturer()),
                      kvp("model", expected_car.get_model()));

    SECTION("Car to BSON works") {
        // Car -> BSON
        bsoncxx::document::value test_value{expected_car};
        auto test_view = test_value.view();

        REQUIRE(string::to_string(test_view["model"].get_string().value) ==
                expected_car.get_model());
        REQUIRE(string::to_string(test_view["manufacturer"].get_string().value) ==
                expected_car.get_manufacturer());
        REQUIRE(test_view == expected_value.view());
    }

    SECTION("BSON to Car works") {
        // BSON -> Car
        test::Car test_car = expected_value.get<test::Car>();
        compare_car_with_car(test_car, expected_car);
    }
}

TEST_CASE("Convert between Person struct and BSON object") {
    const int fave_nums_size = 4;
    int fave_nums[fave_nums_size] = {1, 2, 3, 4};

    test::Car expected_car = {"Knightmare", "Shinkiro"};
    test::Person expected_person{"Lelouch", "Lamperouge", 18, &expected_car, {1, 2, 3, 4}};
    bsoncxx::document::value expected_doc =
        make_document(kvp("first_name", expected_person.first_name),
                      kvp("last_name", expected_person.last_name),
                      kvp("age", expected_person.age),
                      kvp("car", document::value(expected_car)),
                      kvp("fave_numbers", create_array(fave_nums, fave_nums_size)));
    auto expected_view = expected_doc.view();

    SECTION("Conversion from Person struct to document::value works") {
        // Person -> BSON object
        document::value test_value{expected_person};
        auto test_view = test_value.view();

        // Check primitive values
        REQUIRE(string::to_string(test_view["first_name"].get_string().value) ==
                expected_person.first_name);
        REQUIRE(string::to_string(test_view["last_name"].get_string().value) ==
                expected_person.last_name);
        REQUIRE(test_view["age"].get_int32() == expected_person.age);

        // Check nested object
        document::value expected_car_value{expected_car};
        REQUIRE(test_view["car"].get_document().view() == expected_car_value.view());

        // Check array
        array::view test_arr = test_view["fave_numbers"].get_array().value;
        for (int i = 0; i < fave_nums_size; i++) {
            REQUIRE(test_arr[i].get_int32().value == expected_person.fave_numbers[i]);
        }

        REQUIRE(test_view == expected_view);
    }

    SECTION("Conversion from BSON object to Person struct works") {
        // BSON object -> Person
        test::Person test_person = expected_doc.get<test::Person>();
        compare_person_with_person(test_person, expected_person);
    }
}

TEST_CASE("Test out different BSON types for serialization") {
    // Create nested objects
    test::Car expected_car = {"Test", "Car"};
    test::Person expected_person1 = {"Alice", "Bob", 42, &expected_car, {1, 2, 3, 4}};
    test::Person expected_person2 = {"Bob", "Alice", 24, &expected_car, {4, 3, 2, 1}};
    test::Person expected_person3 = {"Alib", "Boce", 0, &expected_car, {1, 3, 4, 2}};

    // Create array
    const int people_size = 3;
    test::Person people[people_size] = {expected_person1, expected_person2, expected_person3};

    // Create expected Types_Test_Object
    document::value expected_person_doc{expected_person1};
    test::Types_Test_Object expected_tto = {4.2,
                                            "utf8",
                                            false,
                                            "CODE",
                                            "SYMBOL",
                                            32,
                                            64,
                                            decimal128{123, 123},
                                            "^foo|bar$",
                                            "i",
                                            100,
                                            1000,
                                            expected_person1};
    expected_tto.initialize_people_array(people);

    // Expected document::value to test against
    bsoncxx::document::value expected_value = make_document(
        kvp("_double", types::b_double{expected_tto.getDouble()}),
        kvp("_utf8", types::b_utf8{expected_tto.getUtf8()}),
        kvp("_bool", types::b_bool{expected_tto.getBool()}),
        kvp("_code", types::b_code{expected_tto.getCode()}),
        kvp("_symbol", types::b_symbol{expected_tto.getSymbol()}),
        kvp("_int32", types::b_int32{expected_tto.getInt32()}),
        kvp("_int64", types::b_int64{expected_tto.getInt64()}),
        kvp("_decimal128", types::b_decimal128{expected_tto.getDecimal128()}),
        kvp("_regex",
            types::b_regex{expected_tto.getRegexString(), expected_tto.getRegexOptions()}),
        kvp("_timestamp",
            types::b_timestamp{expected_tto.getTimestampIncrement(),
                               expected_tto.getTimestampTimestamp()}),
        kvp("_person", document::value(expected_person1)),
        kvp("_people", create_array(people, people_size)));

    SECTION("Types_Test_Object to BSON object serialization") {
        bsoncxx::document::value test_value{expected_tto};
        REQUIRE(test_value.view() == expected_value.view());
    }

    SECTION("BSON object to Types_Test_Object serialization") {
        test::Types_Test_Object test_tto = expected_value.get<test::Types_Test_Object>();

        // BSON types with only one member field
        REQUIRE(test_tto.getDouble() == expected_tto.getDouble());
        REQUIRE(test_tto.getUtf8() == expected_tto.getUtf8());
        REQUIRE(test_tto.getBool() == expected_tto.getBool());
        REQUIRE(test_tto.getCode() == expected_tto.getCode());
        REQUIRE(test_tto.getSymbol() == expected_tto.getSymbol());
        REQUIRE(test_tto.getInt32() == expected_tto.getInt32());
        REQUIRE(test_tto.getInt64() == expected_tto.getInt64());
        REQUIRE(test_tto.getDecimal128() == expected_tto.getDecimal128());

        // BSON types with more than one member field
        REQUIRE(test_tto.getRegexString() == expected_tto.getRegexString());
        REQUIRE(test_tto.getRegexOptions() == expected_tto.getRegexOptions());
        REQUIRE(test_tto.getTimestampIncrement() == expected_tto.getTimestampIncrement());
        REQUIRE(test_tto.getTimestampTimestamp() == expected_tto.getTimestampTimestamp());

        // Check nested objects
        compare_person_with_person(test_tto.getPerson(), expected_tto.getPerson());

        // Check people array
        for (int i = 0; i < people_size; i++) {
            compare_person_with_person(test_tto.getPersonInArray(i), people[i]);
        }
    }
}
}  // namespace
