#pragma once

#include <mw/common/Logger.h>
#include <shared_mutex>

class Logger
{
public:
    static Logger& GetInstance();

    void StartLogger(const std::function<void(const std::string&)>& log_callback);

    void StopLogger();

    void Log(
        const LoggerAPI::LogFile file,
        const LoggerAPI::LogLevel logLevel,
        const std::string& eventText
    ) noexcept;

private:
    Logger();

    mutable std::shared_mutex m_mutex;

    std::function<void(const std::string&)> m_callback;
};