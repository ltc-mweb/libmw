#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Traits
{
    class IJsonable
    {
    public:
        virtual ~IJsonable() = default;

        virtual json ToJSON() const noexcept = 0;
    };
}

template <typename BasicJsonType,
    typename T, typename SFINAE = typename std::enable_if_t<std::is_base_of_v<Traits::IJsonable, T>>>
static void to_json(BasicJsonType& j, const T& value) {
    j = value.ToJSON();
}

template <typename BasicJsonType,
    typename T, typename SFINAE = typename std::enable_if_t<std::is_base_of_v<Traits::IJsonable, T>>>
static void to_json(BasicJsonType& j, const std::shared_ptr<const T>& value) {
    assert(value != nullptr);
    j = value->ToJSON();
}

template <typename BasicJsonType,
    typename T, typename SFINAE = typename std::enable_if_t<std::is_base_of_v<Traits::IJsonable, T>>>
static void to_json(BasicJsonType& j, const std::unique_ptr<const T>& value) {
    assert(value != nullptr);
    j = value->ToJSON();
}

template <typename BasicJsonType,
    typename T, typename SFINAE = typename std::enable_if_t<std::is_base_of_v<Traits::IJsonable, T>>>
static void from_json(const BasicJsonType& j, T& value) {
    value = T::FromJSON(Json(j));
}

template <typename BasicJsonType,
    typename T, typename SFINAE = typename std::enable_if_t<std::is_base_of_v<Traits::IJsonable, T>>>
static void from_json(const BasicJsonType& j, std::shared_ptr<const T>& value) {
    value = T::FromJSON(Json(j));
}

template <typename BasicJsonType,
    typename T, typename SFINAE = typename std::enable_if_t<std::is_base_of_v<Traits::IJsonable, T>>>
static void from_json(const BasicJsonType& j, std::unique_ptr<const T>& value) {
    value = T::FromJSON(Json(j));
}

#include <mw/serialization/Json.h>