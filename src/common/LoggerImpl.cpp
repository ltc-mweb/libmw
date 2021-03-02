#include "LoggerImpl.h"
#include "ThreadManagerImpl.h"

#include <mw/file/FilePath.h>
#include <random>

Logger& Logger::GetInstance()
{
    static Logger instance;
    return instance;
}

void null_logger(const std::string&) {}

Logger::Logger()
    : m_callback(null_logger) { }

static std::string random_string(size_t length)
{
    static auto& chrs = "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    thread_local static std::mt19937 rg{ std::random_device{}() };
    thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

    std::string s;
    s.reserve(length);

    while (length--) {
        s += chrs[pick(rg)];
    }

    return s;
}

void Logger::StartLogger(const std::function<void(const std::string&)>& log_callback)
{
    std::unique_lock<std::shared_mutex> write_lock(m_mutex);
    m_callback = log_callback;
}

void Logger::StopLogger()
{

}

void Logger::Log(
    const LoggerAPI::LogFile file,
    const LoggerAPI::LogLevel logLevel,
    const std::string& eventText) noexcept
{
    std::shared_lock<std::shared_mutex> read_lock(m_mutex);

    std::string eventTextClean = eventText;
    size_t newlinePos = eventTextClean.find("\n");
    while (newlinePos != std::string::npos)
    {
        if (eventTextClean.size() > newlinePos + 2)
        {
            eventTextClean.erase(newlinePos, 2);
        }
        else
        {
            eventTextClean.erase(newlinePos, 1);
        }

        newlinePos = eventTextClean.find("\n");
    }

    const std::string threadName = ThreadManager::GetInstance().GetCurrentThreadName();
    if (!threadName.empty())
    {
        eventTextClean = threadName + " " + eventTextClean;
    }

    m_callback(eventTextClean);
}

namespace LoggerAPI
{
    LOGGER_API void Initialize(const std::function<void(const std::string&)>& log_callback)
    {
        if (log_callback) {
            Logger::GetInstance().StartLogger(log_callback);
        } else {
            Logger::GetInstance().StartLogger(null_logger);
        }
    }

    LOGGER_API void Shutdown()
    {
        Logger::GetInstance().StopLogger();
    }

    LOGGER_API void Flush()
    {

    }

    LOGGER_API void Log(
        const LogFile file,
        const LogLevel logLevel,
        const std::string& function,
        const size_t line,
        const std::string& message) noexcept
    {
        const std::string formatted = function + "(" + std::to_string(line) + ") - " + message;
        Logger::GetInstance().Log(file, logLevel, formatted);
    }
}