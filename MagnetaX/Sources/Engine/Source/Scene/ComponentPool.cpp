// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "ComponentPool.h"
#include <new>

ComponentPool::ComponentPool(usize size, usize align, MoveConstructFn moveConstruct, DestroyFn destroy)
{
    componentSize = size;
    componentAlign = align;
    moveConstructFn = moveConstruct;
    destroyFn = destroy;
}

ComponentPool::~ComponentPool()
{
    for (usize i = 0; i < entities.Size(); ++i) destroyFn(data + i * componentSize);

    if (data) ::operator delete(data, std::align_val_t(componentAlign));
}

void* ComponentPool::Insert(uint32 entity, void* component)
{
    if (Has(entity)) return nullptr;

    if (entities.Size() >= capacity) Grow();

    const usize denseIndex = entities.Size();
    entities.Insert(entity);

    std::byte* dest = data + denseIndex * componentSize;
    moveConstructFn(dest, component);

    return dest;
}

bool ComponentPool::Remove(uint32 entity)
{
    if (!entities.Has(entity)) return false;

    const usize denseIndex = entities.IndexOf(entity);
    const usize lastDenseIndex = entities.Size() - 1;
    void* component = data + denseIndex * componentSize;

    destroyFn(component);

    if (denseIndex != lastDenseIndex)
    {
        void* lastComponent = data + lastDenseIndex * componentSize;

        moveConstructFn(component, lastComponent);
        destroyFn(lastComponent);
    }

    entities.Remove(entity);

    return true;
}

void* ComponentPool::Get(uint32 entity)
{
    if (!entities.Has(entity)) return nullptr;

    const usize denseIndex = entities.IndexOf(entity);

    return data + denseIndex * componentSize;
}

void ComponentPool::Grow()
{
    const usize newCapacity = capacity == 0 ? 4 : capacity * 2;

    std::byte* newData = static_cast<std::byte*>(::operator new(newCapacity * componentSize, std::align_val_t(componentAlign)));

    for (usize i = 0; i < entities.Size(); ++i)
    {
        void* oldComponent = data + i * componentSize;
        void* newComponent = newData + i * componentSize;

        moveConstructFn(newComponent, oldComponent);
        destroyFn(oldComponent);
    }

    if (data) ::operator delete(data, std::align_val_t(componentAlign));

    data = newData;
    capacity = newCapacity;
}
