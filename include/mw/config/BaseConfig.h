#pragma once

#include <mw/exceptions/FatalException.h>
#include <unordered_map>
#include <shared_mutex>
#include <string>

class BaseConfig
{
public:
    BaseConfig(std::unordered_map<std::string, std::string>&& options)
        : m_options(std::move(options)) { }

    std::string Get(const std::string& key, const std::string& defaultValue = "") const noexcept
    {
        std::shared_lock<std::shared_mutex> readLock(m_mutex);

        auto iter = m_options.find(key);
        return iter != m_options.end() ? iter->second : defaultValue;
    }

    std::string GetRequired(const std::string& key) const
    {
        std::shared_lock<std::shared_mutex> readLock(m_mutex);

        auto iter = m_options.find(key);
        if (iter == m_options.end()) {
            ThrowFatal_F("Config option '{}' is required", key);
        }

        return iter->second;
    }

    void Set(const std::string& key, const std::string& value) noexcept
    {
        std::unique_lock<std::shared_mutex> writeLock(m_mutex);

        m_options[key] = value;
    }

private:
    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::string, std::string> m_options;
};