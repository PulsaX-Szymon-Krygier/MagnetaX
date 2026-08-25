// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "../VulkanMinimal.h"

#include <MX/Core/Math/Matrix.h>

class VulkanMesh;
class VulkanMaterial;

struct VulkanDrawItem
{
    VulkanMesh* mesh = nullptr;
    VulkanMaterial* material = nullptr;

    Matrix4f mvp;
    Matrix4f model;
};
