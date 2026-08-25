// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Core/Math/Vector.h>
#include <MX/Core/Math/Quaternion.h>
#include "../Entity.h"
#include <vector>

class TransformComponent
{
public:
    Vector3f position{ 0.0f };
    Quaternion rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
    Vector3f scale{ 1.0f };

    Entity GetParent() const { return parent; }
    std::vector<Entity> GetChildren() const { return children; }

private:
    friend class Scene;

    Entity parent{};
    std::vector<Entity> children;
};
