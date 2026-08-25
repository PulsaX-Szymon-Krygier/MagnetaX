// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "CoreMinimal.h"
#include <string>

struct SemVer
{
    uint16 major = 1;
    uint16 minor = 0;
    uint16 patch = 0;

    SemVer() = default;
    SemVer(uint16 _major, uint16 _minor, uint16 _patch) : major(_major), minor(_minor), patch(_patch) {}

    std::string ToString() const
    {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }
};
