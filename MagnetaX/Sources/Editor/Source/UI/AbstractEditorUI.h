// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

class AbstractEditorUI
{
public:
    virtual ~AbstractEditorUI() = default;

    virtual void Draw() = 0;
};
