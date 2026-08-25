// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Core/Math/Matrix.h>
#include "ComponentType.h"
#include <vector>

inline constexpr uint32 MX_INVALID_ENTITY = 0;

class Scene;

class Entity
{
public:
    Entity() = default;

    template<ComponentType T, typename... Args>
    T& AddComponent(Args&&... args);

    template<ComponentType T>
    T* GetComponent();

    template<ComponentType T>
    bool HasComponent() const;

    template<ComponentType T>
    bool RemoveComponent();

    uint32 GetID() const { return id; }

    bool SetParent(Entity parent);
    Entity GetParent() const;
    bool ClearParent();

    std::vector<Entity> GetChildren() const;

    Matrix4f GetWorldMatrix() const;

private:
    friend class Scene;

    Entity(uint32 _id, Scene* _scene) : id(_id), scene(_scene) {}

    uint32 id = MX_INVALID_ENTITY;
    Scene* scene = nullptr;
};
