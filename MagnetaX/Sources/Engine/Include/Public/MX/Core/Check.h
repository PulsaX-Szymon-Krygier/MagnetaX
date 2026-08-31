// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <source_location>
#include <string_view>

[[noreturn]] void MXCheckFailed(std::string_view condition, std::string_view message, std::source_location location = std::source_location::current());

#define MX_CHECK(condition, message) \
    do \
    { \
        if (!(condition)) MXCheckFailed(#condition, message); \
    } while (false)
