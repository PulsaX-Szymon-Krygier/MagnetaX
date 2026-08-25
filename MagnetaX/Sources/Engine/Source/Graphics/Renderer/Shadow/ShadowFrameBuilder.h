// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Graphics/Renderer/Shadow/ShadowConfig.h>
#include "ShadowFrameData.h"

struct RenderSceneData;

ShadowFrameData BuildShadowFrameData(const RenderSceneData& sceneData, const ShadowConfig& config);
