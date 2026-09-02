// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/Math/Size.h>
#include "RenderSceneData.h"

class Scene;

RenderSceneData BuildRenderSceneData(Scene* scene, const Size2i& renderSize, const Vector2f& projectionJitter);
