// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Graphics/Resources/ImageFormat.h>

struct UITextureHandle
{
    uint64 id = 0;

    bool IsValid() const { return id != 0; }

    explicit operator bool() const { return IsValid(); }

    bool operator==(const UITextureHandle&) const = default;
};

struct UITextureCreateInfo
{
    const uint8* pixels = nullptr;

    uint32 width = 0;
    uint32 height = 0;

    ImageFormat format = ImageFormat::UNKNOWN;
};
