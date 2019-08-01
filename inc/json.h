#pragma once

#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace json
{
    struct value;

    enum class value_type
    {
        null,
        boolean,
        number,
        string,
        array,
        object,
    };

    using null_t = std::nullptr_t;
    using boolean_t = bool;
    using number_t = double;
    using string_t = std::string;
    template <typename T = value> using array_t = std::vector<T>;
    using object_t = std::map<string_t, value, std::less<>>;

    template <typename T>
    using optional_t = std::optional<T>;

    struct value
    {
        template <typename T>
        value(T&& val) : data(std::forward<T>(val)) {}

        value_type type() const noexcept
        {
            return static_cast<value_type>(data.index());
        }

        template <typename T> T& get() { return std::get<T>(data); }
        template <typename T> const T& get() const { return std::get<T>(data); }

        boolean_t boolean() const { return get<boolean_t>(); }
        number_t number() const { return get<number_t>(); }
        string_t& string() { return get<string_t>(); }
        const string_t& string() const { return get<string_t>(); }
        array_t<>& array() { return get<array_t<>>(); }
        const array_t<>& array() const { return get<array_t<>>(); }
        object_t& object() { return get<object_t>(); }
        const object_t& object() const { return get<object_t>(); }

        value& at(std::size_t index) { return array().at(index); }
        const value& at(std::size_t index) const { return array().at(index); }
        value& operator[](std::size_t index) { return array()[index]; }
        const value& operator[](std::size_t index) const { return array()[index]; }

        value* try_get(std::string_view key)
        {
            auto& obj = object();
            auto itr = obj.find(key);
            return (itr == obj.end()) ? nullptr : &itr->second;
        }
        const value* try_get(std::string_view key) const { return const_cast<value*>(this)->try_get(key); }
        value& get(std::string_view key)
        {
            if (auto ptr = try_get(key))
            {
                return *ptr;
            }

            std::string msg = "Key '";
            msg.append(key);
            msg += "' is not present in the object";
            throw std::runtime_error(std::move(msg));
        }
        const value& get(std::string_view key) const { return const_cast<value*>(this)->get(key); }
        value& operator[](std::string_view key) { return get(key); }
        const value& operator[](std::string_view key) const { return get(key); }

        // NOTE: Order is such that 'value_type' correlates to the index
        std::variant<
            null_t,
            boolean_t,
            number_t,
            string_t,
            array_t<>,
            object_t> data;
    };
}
