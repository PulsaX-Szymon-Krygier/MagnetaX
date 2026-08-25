// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

class AssetManager;

class AbstractAsset
{
public:
    virtual ~AbstractAsset() = default;

protected:
    AbstractAsset() = default;

    virtual bool Load() = 0;
    virtual void Unload() = 0;

private:
    friend class AssetManager;
};
