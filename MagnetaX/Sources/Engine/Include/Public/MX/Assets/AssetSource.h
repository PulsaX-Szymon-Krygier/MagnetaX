// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <string>

class AssetSource
{
public:
    explicit AssetSource(std::string _path);

    const std::string& GetPath() const { return path; }

private:
    std::string path;
};
