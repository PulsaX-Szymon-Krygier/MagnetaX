// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Core/Check.h>
#include <MX/Core/Platform.h>
#include <cstdio>
#include <cstdlib>

#if MX_PLATFORM_WINDOWS
#include <Windows.h>
#endif

static void WriteCheckOutput(const char* output)
{
    #if MX_PLATFORM_WINDOWS
    if (IsDebuggerPresent())
    {
        OutputDebugStringA(output);
        return;
    }
    #endif

    std::fputs(output, stderr);
    std::fflush(stderr);
}

[[noreturn]] void MXCheckFailed(std::string_view condition, std::string_view message, std::source_location location)
{
    char output[4096];

    const int result = std::snprintf(output, sizeof(output),
        "\nMagnetaX fatal check failed\n"
        "Condition: %.*s\n"
        "Message:   %.*s\n"
        "Location:  %s:%u\n"
        "Function:  %s\n\n", static_cast<int>(condition.size()), condition.data(), static_cast<int>(message.size()),
        message.data(), location.file_name(), static_cast<unsigned int>(location.line()), location.function_name());

    if (result < 0)
    {
        WriteCheckOutput("\nMagnetaX fatal check failed.\n");
        std::abort();
    }

    WriteCheckOutput(output);
    std::abort();
}
