// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Scene/Entity.h>
#include "../AbstractEditorUI.h"

class ImGuiEditorUI : public AbstractEditorUI
{
public:
    void Draw(EditorContext& context) override;

private:
    void DrawEntityNode(Entity entity, EditorContext& context);
};
