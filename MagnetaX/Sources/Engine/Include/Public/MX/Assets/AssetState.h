// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

enum class AssetState
{
    UNLOADED,
    LOADING,
    LOADED,
    FAILED,
    INVALID
};
