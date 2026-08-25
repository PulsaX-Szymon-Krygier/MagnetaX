// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <Core/Containers/SparseSet.h>
#include <cstddef>

class ComponentPool
{
public:
    using MoveConstructFn = void(*)(void* dest, void* src);
    using DestroyFn = void(*)(void* object);

    ComponentPool(usize size, usize align, MoveConstructFn moveConstruct, DestroyFn destroy);
    ComponentPool(const ComponentPool&) = delete;
    ComponentPool& operator=(const ComponentPool&) = delete;

    ~ComponentPool();

    void* Insert(uint32 entity, void* component);
    bool Remove(uint32 entity);

    bool Has(uint32 entity) const { return entities.Has(entity); }
    void* Get(uint32 entity);

    usize Size() const { return entities.Size(); }

    uint32 EntityAt(usize index) const { return entities.KeyAt(index); }
    void* ComponentAt(usize index) { return data + index * componentSize; }

private:
    usize componentSize;
    usize componentAlign;

    MoveConstructFn moveConstructFn;
    DestroyFn destroyFn;

    SparseSet<uint32> entities;

    std::byte* data = nullptr;
    usize capacity = 0;

    void Grow();
};
