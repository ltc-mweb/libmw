#include "LoggerImpl.h"
#include "ThreadManagerImpl.h"

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <mw/file/FilePath.h>

Logger& Logger::GetInstance()
{
    static Logger instance;
    return instance;
}

// TODO: Make asynchronous
void Logger::StartLogger(const FilePath& logDirectory, const spdlog::level::level_enum& logLevel)
{
    logDirectory.CreateDirIfMissing();

    {
        const FilePath logPath = logDirectory.GetChild("Node.log");
        m_pNodeLogger = spdlog::rotating_logger_mt("NODE", logPath.ToString(), 5 * 1024 * 1024, 10);
        //m_pNodeLogger = spdlog::create_async<spdlog::sinks::rotating_file_sink_mt>("NODE", sink, 32768, spdlog::async_overflow_policy::block, nullptr, std::chrono::seconds(5));
        spdlog::set_pattern("[%D %X.%e%z] [%l] %v");
        if (m_pNodeLogger != nullptr)
        {
            m_pNodeLogger->set_level(logLevel);
        }
    }
    {
        const FilePath logPath = logDirectory.GetChild("Wallet.log");
        m_pWalletLogger = spdlog::rotating_logger_mt("WALLET", logPath.ToString(), 5 * 1024 * 1024, 10);
        //auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logPath.ToString(), 5 * 1024 * 1024, 10);
        //m_pWalletLogger = spdlog::create_async("WALLET", sink, 8192, spdlog::async_overflow_policy::block, nullptr, std::chrono::seconds(5));
        spdlog::set_pattern("[%D %X.%e%z] [%l] %v");
        if (m_pWalletLogger != nullptr)
        {
            m_pWalletLogger->set_level(logLevel);
        }
    }
}

spdlog::level::level_enum Logger::Convert(LoggerAPI::LogLevel logLevel) noexcept
{
    switch (logLevel)
    {
        case LoggerAPI::LogLevel::TRACE:
            return spdlog::level::trace;
        case LoggerAPI::LogLevel::DEBUG:
            return spdlog::level::debug;
        case LoggerAPI::LogLevel::INFO:
            return spdlog::level::info;
        case LoggerAPI::LogLevel::WARN:
            return spdlog::level::warn;
        case LoggerAPI::LogLevel::ERR:
            return spdlog::level::err;
        default:
            return spdlog::level::off;
    }
}

LoggerAPI::LogLevel Logger::Convert(spdlog::level::level_enum logLevel) noexcept
{
    switch (logLevel)
    {
        case spdlog::level::trace:
            return LoggerAPI::LogLevel::TRACE;
        case spdlog::level::debug:
            return LoggerAPI::LogLevel::DEBUG;
        case spdlog::level::info:
            return LoggerAPI::LogLevel::INFO;
        case spdlog::level::warn:
            return LoggerAPI::LogLevel::WARN;
        case spdlog::level::err:
            return LoggerAPI::LogLevel::ERR;
        default:
            return LoggerAPI::LogLevel::NONE;
    }
}

void Logger::Log(
    const LoggerAPI::LogFile file,
    const LoggerAPI::LogLevel logLevel,
    const std::string& eventText) noexcept
{
    auto pLogger = GetLogger(file);
    if (pLogger != nullptr)
    {
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

        pLogger->log(Convert(logLevel), eventTextClean);
    }
}

LoggerAPI::LogLevel Logger::GetLogLevel(const LoggerAPI::LogFile file) const noexcept
{
    auto pLogger = GetLogger(file);
    if (pLogger != nullptr)
    {
        return Convert(pLogger->level());
    }

    return LoggerAPI::LogLevel::NONE;
}

void Logger::Flush()
{
    if (m_pNodeLogger != nullptr)
    {
        m_pNodeLogger->flush();
    }

    if (m_pWalletLogger != nullptr)
    {
        m_pWalletLogger->flush();
    }
}

std::shared_ptr<spdlog::logger> Logger::GetLogger(const LoggerAPI::LogFile file) noexcept
{
    if (file == LoggerAPI::LogFile::WALLET)
    {
        return m_pWalletLogger;
    }
    else
    {
        return m_pNodeLogger;
    }
}

std::shared_ptr<const spdlog::logger> Logger::GetLogger(const LoggerAPI::LogFile file) const noexcept
{
    if (file == LoggerAPI::LogFile::WALLET)
    {
        return m_pWalletLogger;
    }
    else
    {
        return m_pNodeLogger;
    }
}

namespace LoggerAPI
{
    LOGGER_API void Initialize(const std::string& logDirectory, const std::string& logLevel)
    {
        spdlog::level::level_enum logLevelEnum = spdlog::level::level_enum::debug;
        if (logLevel == "TRACE")
        {
            logLevelEnum = spdlog::level::level_enum::trace;
        }
        else if (logLevel == "DEBUG")
        {
            logLevelEnum = spdlog::level::level_enum::debug;
        }
        else if (logLevel == "INFO")
        {
            logLevelEnum = spdlog::level::level_enum::info;
        }
        else if (logLevel == "WARN")
        {
            logLevelEnum = spdlog::level::level_enum::warn;
        }
        else if (logLevel == "ERROR")
        {
            logLevelEnum = spdlog::level::level_enum::err;
        }

        Logger::GetInstance().StartLogger(logDirectory, logLevelEnum);
    }

    LOGGER_API void Flush()
    {
        Logger::GetInstance().Flush();
    }

    LOGGER_API LogLevel GetLogLevel(const LogFile file) noexcept
    {
        return Logger::GetInstance().GetLogLevel(file);
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