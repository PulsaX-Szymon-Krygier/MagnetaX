// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Core/Math/Size.h>

Size2i::Size2i(uint32 value) : width(value), height(value) {}

Size2i::Size2i(uint32 _width, uint32 _height) : width(_width), height(_height) {}

bool Size2i::operator==(const Size2i& r) const
{
    return width == r.width && height == r.height;
}

bool Size2i::operator!=(const Size2i& r) const
{
    return !(*this == r);
}

Size3i::Size3i(uint32 value) : width(value), height(value), depth(value) {}

Size3i::Size3i(const Size2i& size2, uint32 _depth) : width(size2.width), height(size2.height), depth(_depth) {}

Size3i::Size3i(uint32 _width, uint32 _height, uint32 _depth) : width(_width), height(_height), depth(_depth) {}

bool Size3i::operator==(const Size3i& r) const
{
    return width == r.width && height == r.height && depth == r.depth;
}

bool Size3i::operator!=(const Size3i& r) const
{
    return !(*this == r);
}
