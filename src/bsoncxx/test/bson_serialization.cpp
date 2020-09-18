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

#include <iostream>
#include <string>
#include <vector>

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/test_util/catch.hh>

using namespace bsoncxx;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::kvp;

namespace {

namespace test {

struct Person {
    std::string first_name;
    std::string last_name;
    int age;
//    std::vector<std::string> favorite_colors;
};

void to_bson(const Person &person , document::value &bson_object) {
    // Include the members and values of the Person you'd like to turn into a BSON object
    bson_object = make_document(
        kvp("first_name", person.first_name),
        kvp("last_name", person.last_name),
        kvp("age", person.age)
    );
}
//void to_bson() {
//    std::cout << "Hello WORLD\n";
//}

//void from_bson(const Person &person, document::value &bson_object) {
//    // Include the data you'd like to get from the BSON object into your Person object
//    // Change input to document::view instead to avoid all of the calls to .view()
//    bson_object.view()["first_name"].give_value(person.first_name);
//    bson_object.view()["last_name"].give_value(person.last_name);
//    bson_object.view()["age"].give_value(person.age);
//    bson_object.view()["favorite_colors"].give_value(person.favorite_colors);
//}
//void from_bson() {
//    std::cout << "Hello WORLD\n";
//}

} // namespace test_bson_serialization

TEST_CASE() {
    test::Person expected_person{
        "Lelouch",
        "Lamperouge",
        18,
    };
    document::value doc = builder::basic::make_document(
        kvp("first_name", "Lelouch"),
        kvp("last_name", "Lamperouge"),
        kvp("age", 18)
    );

    // Person -> BSON document
//    document::value bson_object = document::value{expected_person};

    // BSON document -> Person
//    test::Person actual_person = bson_object;
    // Maybe try:
//     test::Person actual_person = doc.get<test::Person>();
//    doc.get<test::Person>();
    // Where bson_object.get<T>() is the function that calls test::from_bson() ?
//
//    REQUIRE(actual_person.first_name == expected_person.first_name);
//    REQUIRE(actual_person.last_name == expected_person.last_name);
//    REQUIRE(actual_person.age == expected_person.age);
//    REQUIRE(actual_person.favorite_colors == expected_person.favorite_colors);
}

} // namespace