// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Scene/Entity.h>
#include <MX/Core/Math/Vector.h>
#include <unordered_map>

class Scene;

struct EditorContext
{
    Scene* scene = nullptr;
    Entity selectedEntity;

    std::unordered_map<uint32, Vector3f> rotationEulerHints;
};
