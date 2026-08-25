// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Scene/Entity.h>
#include <MX/Scene/Scene.h>

bool Entity::SetParent(Entity parent)
{
    if (!scene) return false;

    return scene->SetParent(*this, parent);
}

Entity Entity::GetParent() const
{
    if (!scene) return {};

    return scene->GetParent(*this);
}

bool Entity::ClearParent()
{
    if (!scene) return false;

    return scene->ClearParent(*this);
}

std::vector<Entity> Entity::GetChildren() const
{
    if (!scene) return {};

    return scene->GetChildren(*this);
}

Matrix4f Entity::GetWorldMatrix() const
{
    if (!scene) return Matrix4f::Identity();

    return scene->GetWorldMatrix(*this);
}
