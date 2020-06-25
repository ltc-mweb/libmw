#pragma once

#include <mw/common/ImportExport.h>
#include <mw/util/StringUtil.h>
#include <mw/file/FilePath.h>

//#ifdef MW_COMMON
//#define LOGGER_API EXPORT
//#else
//#define LOGGER_API IMPORT
//#endif

#define LOGGER_API

namespace LoggerAPI
{
    enum class LogFile
    {
        NODE,
        WALLET
    };

    enum LogLevel : uint8_t
    {
        NONE = 0,
        TRACE = 1,
        DEBUG = 2,
        INFO = 3,
        WARN = 4,
        ERR = 5
    };

    LOGGER_API void Initialize(const FilePath& logDirectory, const std::string& logLevel);
    LOGGER_API void Flush();

    LOGGER_API LogLevel GetLogLevel(const LogFile file) noexcept;

    LOGGER_API void Log(
        const LogFile file,
        const LogLevel logLevel,
        const std::string& function,
        const size_t line,
        const std::string& message
    ) noexcept;
}

template<typename ... Args>
static void LOG_F(
    const LoggerAPI::LogFile file,
    const LoggerAPI::LogLevel logLevel,
    const std::string& function,
    const size_t line,
    const char* format,
    const Args& ... args) noexcept
{
    if (logLevel >= GetLogLevel(file))
    {
        try
        {
            std::string message = StringUtil::Format(format, args...);
            Log(file, logLevel, function, line, message);
        }
        catch (std::exception&)
        {
            // Logger failure should not disrupt program flow
        }
    }
}

// Node Logger
#define LOG_TRACE(message) LoggerAPI::Log(LoggerAPI::LogFile::NODE, LoggerAPI::LogLevel::TRACE, __FUNCTION__, __LINE__, message)
#define LOG_DEBUG(message) LoggerAPI::Log(LoggerAPI::LogFile::NODE, LoggerAPI::LogLevel::DEBUG, __FUNCTION__, __LINE__, message)
#define LOG_INFO(message) LoggerAPI::Log(LoggerAPI::LogFile::NODE, LoggerAPI::LogLevel::INFO, __FUNCTION__, __LINE__, message)
#define LOG_WARNING(message) LoggerAPI::Log(LoggerAPI::LogFile::NODE, LoggerAPI::LogLevel::WARN, __FUNCTION__, __LINE__, message)
#define LOG_ERROR(message) LoggerAPI::Log(LoggerAPI::LogFile::NODE, LoggerAPI::LogLevel::ERR, __FUNCTION__, __LINE__, message)

#define LOG_TRACE_F(message, ...) LOG_F(LoggerAPI::LogFile::NODE, LoggerAPI::LogLevel::TRACE, __FUNCTION__, __LINE__, message, __VA_ARGS__)
#define LOG_DEBUG_F(message, ...) LOG_F(LoggerAPI::LogFile::NODE, LoggerAPI::LogLevel::DEBUG, __FUNCTION__, __LINE__, message, __VA_ARGS__)
#define LOG_INFO_F(message, ...) LOG_F(LoggerAPI::LogFile::NODE, LoggerAPI::LogLevel::INFO, __FUNCTION__, __LINE__, message, __VA_ARGS__)
#define LOG_WARNING_F(message, ...) LOG_F(LoggerAPI::LogFile::NODE, LoggerAPI::LogLevel::WARN, __FUNCTION__, __LINE__, message, __VA_ARGS__)
#define LOG_ERROR_F(message, ...) LOG_F(LoggerAPI::LogFile::NODE, LoggerAPI::LogLevel::ERR, __FUNCTION__, __LINE__, message, __VA_ARGS__)

// Wallet Logger
#define WALLET_TRACE(message) LoggerAPI::Log(LoggerAPI::LogFile::WALLET, LoggerAPI::LogLevel::TRACE, __FUNCTION__, __LINE__, message)
#define WALLET_DEBUG(message) LoggerAPI::Log(LoggerAPI::LogFile::WALLET, LoggerAPI::LogLevel::DEBUG, __FUNCTION__, __LINE__, message)
#define WALLET_INFO(message) LoggerAPI::Log(LoggerAPI::LogFile::WALLET, LoggerAPI::LogLevel::INFO, __FUNCTION__, __LINE__, message)
#define WALLET_WARNING(message) LoggerAPI::Log(LoggerAPI::LogFile::WALLET, LoggerAPI::LogLevel::WARN, __FUNCTION__, __LINE__, message)
#define WALLET_ERROR(message) LoggerAPI::Log(LoggerAPI::LogFile::WALLET, LoggerAPI::LogLevel::ERR, __FUNCTION__, __LINE__, message)

#define WALLET_TRACE_F(message, ...) LOG_F(LoggerAPI::LogFile::WALLET, LoggerAPI::LogLevel::TRACE, __FUNCTION__, __LINE__, message, __VA_ARGS__)
#define WALLET_DEBUG_F(message, ...) LOG_F(LoggerAPI::LogFile::WALLET, LoggerAPI::LogLevel::DEBUG, __FUNCTION__, __LINE__, message, __VA_ARGS__)
#define WALLET_INFO_F(message, ...) LOG_F(LoggerAPI::LogFile::WALLET, LoggerAPI::LogLevel::INFO, __FUNCTION__, __LINE__, message, __VA_ARGS__)
#define WALLET_WARNING_F(message, ...) LOG_F(LoggerAPI::LogFile::WALLET, LoggerAPI::LogLevel::WARN, __FUNCTION__, __LINE__, message, __VA_ARGS__)
#define WALLET_ERROR_F(message, ...) LOG_F(LoggerAPI::LogFile::WALLET, LoggerAPI::LogLevel::ERR, __FUNCTION__, __LINE__, message, __VA_ARGS__)