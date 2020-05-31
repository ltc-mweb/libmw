#pragma once

#include <mw/traits/Jsonable.h>
#include <mw/traits/Printable.h>
#include <tl/optional.hpp>
#include <type_traits>

class Json : public Traits::IPrintable
{
public:
    Json(json&& json) : m_json(std::move(json)) { }
    Json(const json& json) : m_json(json) { }
    virtual ~Json() = default;

    template<class T>
    tl::optional<typename std::enable_if_t<std::is_arithmetic_v<T>, T>> Get(const std::string& key) const
    {
        auto iter = m_json.find(key);
        if (iter != m_json.end())
        {
            if (iter->is_number())
            {
                return tl::make_optional<T>(iter->get<T>());
            }
        }

        return tl::nullopt;
    }

    template<class T>
    tl::optional<typename std::enable_if_t<std::is_same_v<bool, T>, T>> Get(const std::string& key) const
    {
        auto iter = m_json.find(key);
        if (iter != m_json.end())
        {
            if (iter->is_bool())
            {
                return tl::make_optional<T>(iter->get<T>());
            }
        }

        return tl::nullopt;
    }

    template<class T>
    tl::optional<typename std::enable_if_t<std::is_base_of_v<T, std::string>, T>> Get(const std::string& key) const
    {
        auto iter = m_json.find(key);
        if (iter != m_json.end())
        {
            if (iter->is_string())
            {
                return tl::make_optional<T>(iter->get<T>());
            }
        }

        return tl::nullopt;
    }

    template<class T>
    tl::optional<typename std::enable_if_t<std::is_same_v<T, json>, T>> Get(const std::string& key) const
    {
        auto iter = m_json.find(key);
        if (iter != m_json.end())
        {
            if (iter->is_object() || iter->is_array())
            {
                return tl::make_optional<T>(*iter);
            }
        }

        return tl::nullopt;
    }

    template<class T>
    tl::optional<typename std::enable_if_t<std::is_same_v<T, Json>, T>> Get(const std::string& key) const
    {
        auto iter = m_json.find(key);
        if (iter != m_json.end())
        {
            if (iter->is_object() || iter->is_array())
            {
                return tl::make_optional<T>(Json(*iter));
            }
        }

        return tl::nullopt;
    }

    template<class T>
    tl::optional<typename std::enable_if_t<std::is_base_of_v<T, Traits::IJsonable>, T>> Get(const std::string& key) const
    {
        auto iter = m_json.find(key);
        if (iter != m_json.end())
        {
            if (iter->is_object() || iter->is_array())
            {
                return tl::make_optional<T>(iter->get<T>());
            }
        }

        return tl::nullopt;
    }

    template<class T>
    T GetOr(const std::string& key, const T& defaultValue) const
    {
        return Get<T>(key).value_or(defaultValue);
    }

    template<class T>
    T GetRequired(const std::string& key) const
    {
        auto iter = m_json.find(key);
        if (iter != m_json.end())
        {
            return iter->get<T>();
        }

        ThrowDeserialization_F("Failed to deserialize {}", key);
    }

    template<class T>
    T Get() const
    {
        return m_json.get<T>();
    }

    std::vector<std::string> GetKeys() const noexcept
    {
        assert(m_json.is_object());
        
        std::vector<std::string> fields;
        for (auto& item : m_json.items())
        {
            fields.push_back(item.key());
        }

        return fields;
    }

    bool Exists(const std::string& key) const noexcept
    {
        return m_json.find(key) != m_json.cend();
    }

    std::string Format() const final { return m_json.dump(); }

private:
    json m_json;
};