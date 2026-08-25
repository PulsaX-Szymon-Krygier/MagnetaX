// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "../CoreMinimal.h"

struct Size2i
{
    uint32 width{ 0 };
    uint32 height{ 0 };

    Size2i() = default;
    Size2i(uint32 value);
    Size2i(uint32 _width, uint32 _height);

    bool operator==(const Size2i& r) const;
    bool operator!=(const Size2i& r) const;
};

struct Size3i
{
    uint32 width{ 0 };
    uint32 height{ 0 };
    uint32 depth{ 0 };

    Size3i() = default;
    Size3i(uint32 value);
    Size3i(const Size2i& size2, uint32 _depth);
    Size3i(uint32 _width, uint32 _height, uint32 _depth);

    bool operator==(const Size3i& r) const;
    bool operator!=(const Size3i& r) const;
};
