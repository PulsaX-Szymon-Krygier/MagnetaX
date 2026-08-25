// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "../CoreMinimal.h"
#include <filesystem>
#include <vector>

struct IOUtils
{
    static std::filesystem::path GetExecutablePath();
    static std::filesystem::path GetExecutableDirectory();
    static bool ReadBinaryFile(const std::filesystem::path& path, std::vector<uint8>& data);
};
