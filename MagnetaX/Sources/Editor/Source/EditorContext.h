// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Scene/Entity.h>

class Scene;

struct EditorContext
{
    Scene* scene = nullptr;
    Entity selectedEntity;
};
