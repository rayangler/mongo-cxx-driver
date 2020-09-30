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

#include <bsoncxx/types/bson_value/view.hpp>

#include <cstdlib>

#include <bsoncxx/exception/error_code.hpp>
#include <bsoncxx/exception/exception.hpp>
#include <bsoncxx/private/libbson.hh>
#include <bsoncxx/private/suppress_deprecation_warnings.hh>
#include <bsoncxx/types/private/convert.hh>

#include <bsoncxx/config/private/prelude.hh>

#define BSONCXX_CITER \
    bson_iter_t iter; \
    bson_iter_init_from_data_at_offset(&iter, raw, length, offset, keylen);

#define BSONCXX_TYPE_CHECK(name)                                                \
    do {                                                                        \
        if (type() != bsoncxx::type::k_##name) {                                \
            throw bsoncxx::exception{error_code::k_need_element_type_k_##name}; \
        }                                                                       \
    } while (0)

namespace bsoncxx {
BSONCXX_INLINE_NAMESPACE_BEGIN
namespace types {
namespace bson_value {

view::view() noexcept : view(nullptr) {}

// Boost doesn't mark the copy constructor and copy-assignment operator of string_ref as noexcept
// so we can't rely on automatic noexcept propagation. It really is though, so it is OK.
#if !defined(BSONCXX_POLY_USE_BOOST)
#define BSONCXX_ENUM(name, val)                                                                \
    view::view(b_##name value) noexcept : _type(static_cast<bsoncxx::type>(val)),              \
                                          _b_##name(std::move(value)) {                        \
        static_assert(std::is_nothrow_copy_constructible<b_##name>::value, "Copy may throw");  \
        static_assert(std::is_nothrow_copy_assignable<b_##name>::value, "Copy may throw");     \
        static_assert(std::is_nothrow_destructible<b_##name>::value, "Destruction may throw"); \
    }
#else
#define BSONCXX_ENUM(name, val)                                                                \
    view::view(b_##name value) noexcept : _type(static_cast<bsoncxx::type>(val)),              \
                                          _b_##name(std::move(value)) {                        \
        static_assert(std::is_nothrow_destructible<b_##name>::value, "Destruction may throw"); \
    }
#endif

#include <bsoncxx/enums/type.hpp>
#undef BSONCXX_ENUM

view::view(const view& rhs) noexcept {
    switch (static_cast<int>(rhs._type)) {
        // CXX-1817; deprecation warning suppressed for get_utf8()
        BSONCXX_SUPPRESS_DEPRECATION_WARNINGS_BEGIN
#define BSONCXX_ENUM(type, val)                      \
    case val:                                        \
        new (&_b_##type) b_##type(rhs.get_##type()); \
        break;
#include <bsoncxx/enums/type.hpp>
#undef BSONCXX_ENUM
        BSONCXX_SUPPRESS_DEPRECATION_WARNINGS_END
    }

    _type = rhs._type;
}

view& view::operator=(const view& rhs) noexcept {
    if (this == &rhs) {
        return *this;
    }

    destroy();

    switch (static_cast<int>(rhs._type)) {
        // CXX-1817; deprecation warning suppressed for get_utf8()
        BSONCXX_SUPPRESS_DEPRECATION_WARNINGS_BEGIN
#define BSONCXX_ENUM(type, val)                      \
    case val:                                        \
        new (&_b_##type) b_##type(rhs.get_##type()); \
        break;
#include <bsoncxx/enums/type.hpp>
#undef BSONCXX_ENUM
        BSONCXX_SUPPRESS_DEPRECATION_WARNINGS_END
    }

    _type = rhs._type;
    return *this;
}

view::~view() {
    destroy();
}

bsoncxx::type view::type() const {
    return _type;
}

#define BSONCXX_ENUM(type, val)                       \
    const types::b_##type& view::get_##type() const { \
        BSONCXX_TYPE_CHECK(type);                     \
        return _b_##type;                             \
    }
#include <bsoncxx/enums/type.hpp>
#undef BSONCXX_ENUM

const b_utf8& view::get_string() const {
    return _b_utf8;
}

view::view(const std::uint8_t* raw,
           std::uint32_t length,
           std::uint32_t offset,
           std::uint32_t keylen) {
    BSONCXX_CITER;

    auto value = bson_iter_value(&iter);

    _init((void*)value);
}

view::view(void* internal_value) noexcept {
    _init(internal_value);
}

void view::_init(void* internal_value) noexcept {
    if (!internal_value) {
        _type = bsoncxx::type::k_null;
        _b_null = bsoncxx::types::b_null{};
        return;
    }

    bson_value_t* v = (bson_value_t*)(internal_value);
    _type = static_cast<bsoncxx::type>(v->value_type);

    switch (_type) {
#define BSONCXX_ENUM(name, val)              \
    case bsoncxx::type::k_##name: {          \
        convert_from_libbson(v, &_b_##name); \
        break;                               \
    }
#include <bsoncxx/enums/type.hpp>
#undef BSONCXX_ENUM
        default:
            BSONCXX_UNREACHABLE;
    }
}

bool operator==(const view& lhs, const view& rhs) {
    if (lhs.type() != rhs.type()) {
        return false;
    }

    switch (static_cast<int>(lhs.type())) {
        // CXX-1817; deprecation warning suppressed for get_utf8()
        BSONCXX_SUPPRESS_DEPRECATION_WARNINGS_BEGIN
#define BSONCXX_ENUM(type, val) \
    case val:                   \
        return lhs.get_##type() == rhs.get_##type();
#include <bsoncxx/enums/type.hpp>
#undef BSONCXX_ENUM
        BSONCXX_SUPPRESS_DEPRECATION_WARNINGS_END
    }

    // Silence compiler warnings about failing to return a value.
    BSONCXX_UNREACHABLE;
}

bool operator!=(const view& lhs, const view& rhs) {
    return !(lhs == rhs);
}

// Serializer functions
void view::_to_field(std::string& object_field) const {
    // Check type() here and use cases of _b_##type where _b_##type.value is a string
    if (_type == bsoncxx::type::k_utf8) {
        object_field = this->_b_utf8.value.to_string();
    } else if (_type == bsoncxx::type::k_code) {
        object_field = this->_b_code.code.to_string();
    } else if (_type == bsoncxx::type::k_symbol) {
        object_field = this->_b_symbol.symbol.to_string();
    }
}

void view::_to_field(int32_t& object_field) const {
    object_field = this->_b_int32.value;
}

void view::_to_field(int64_t& object_field) const {
    object_field = this->_b_int64.value;
}

void view::_to_field(decimal128& object_field) const {
    object_field = this->_b_decimal128.value;
}

void view::_to_field(double& object_field) const {
    object_field = this->_b_double.value;
}

void view::_to_field(bool& object_field) const {
    object_field = this->_b_bool.value;
}

void view::_to_fields(std::string& object_field1, std::string& object_field2) const {
    object_field1 = this->_b_regex.regex.to_string();
    object_field2 = this->_b_regex.options.to_string();
}

void view::_to_fields(uint32_t& object_field1, uint32_t& object_field2) const {
    object_field1 = this->_b_timestamp.increment;
    object_field2 = this->_b_timestamp.timestamp;
}

void view::destroy() noexcept {
    switch (static_cast<int>(_type)) {
#define BSONCXX_ENUM(type, val) \
    case val:                   \
        _b_##type.~b_##type();  \
        break;
#include <bsoncxx/enums/type.hpp>
#undef BSONCXX_ENUM
    }
}

}  // namespace bson_value
}  // namespace types
BSONCXX_INLINE_NAMESPACE_END
}  // namespace bsoncxx
