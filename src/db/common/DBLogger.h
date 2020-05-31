#pragma once

#include <mw/common/Logger.h>
#include <leveldb/env.h>
#include <cstdarg>
#include <cassert>
#include <cstdio>

class DBLogger final : public leveldb::Logger
{
public:
    DBLogger() = default;
    ~DBLogger() override = default;

    void Logv(const char* format, va_list arguments) final
    {
        // We first attempt to print into a stack-allocated buffer. If this attempt
        // fails, we make a second attempt with a dynamically allocated buffer.
        constexpr const int kStackBufferSize = 512;
        char stack_buffer[kStackBufferSize];
        static_assert(sizeof(stack_buffer) == static_cast<size_t>(kStackBufferSize),
            "sizeof(char) is expected to be 1 in C++");

        int dynamic_buffer_size = 0;  // Computed in the first iteration.
        for (int iteration = 0; iteration < 2; ++iteration) {
            const int buffer_size =
                (iteration == 0) ? kStackBufferSize : dynamic_buffer_size;
            char* const buffer =
                (iteration == 0) ? stack_buffer : new char[dynamic_buffer_size];

            int buffer_offset = 0;

            // Print the message into the buffer.
            std::va_list arguments_copy;
            va_copy(arguments_copy, arguments);
            buffer_offset +=
                std::vsnprintf(buffer + buffer_offset, buffer_size - buffer_offset,
                    format, arguments_copy);
            va_end(arguments_copy);

            // The code below may append a newline at the end of the buffer, which
            // requires an extra character.
            if (buffer_offset >= buffer_size - 1) {
                // The message did not fit into the buffer.
                if (iteration == 0) {
                    // Re-run the loop and use a dynamically-allocated buffer. The buffer
                    // will be large enough for the log message, an extra newline and a
                    // null terminator.
                    dynamic_buffer_size = buffer_offset + 2;
                    continue;
                }

                // The dynamically-allocated buffer was incorrectly sized. This should
                // not happen, assuming a correct implementation of (v)snprintf. Fail
                // in tests, recover by truncating the log message in production.
                assert(false);
                buffer_offset = buffer_size - 1;
            }

            assert(buffer_offset <= buffer_size);
            std::string message(buffer);
            LOG_INFO(message);

            if (iteration != 0)
            {
                delete[] buffer;
            }

            break;
        }
    }
};