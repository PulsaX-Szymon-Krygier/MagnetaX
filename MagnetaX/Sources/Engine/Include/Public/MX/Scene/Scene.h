// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/Math/Matrix.h>
#include <MX/Core/Check.h>
#include "Entity.h"
#include "SceneEnvironment.h"
#include <memory>
#include <utility>
#include <new>
#include <string_view>
#include <vector>

class Scene
{
public:
    Scene();

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    ~Scene();

    SceneEnvironment& GetEnvironment();
    const SceneEnvironment& GetEnvironment() const;

    Entity CreateEntity();

    bool SetActiveCamera(Entity entity);
    void ClearActiveCamera();
    Entity GetActiveCamera();

    Entity GetEntityByID(uint32 id);
    std::vector<Entity> GetEntitiesByName(std::string_view name);

    Matrix4f GetWorldMatrix(Entity entity) const;

    template<ComponentType... T>
        requires (sizeof...(T) > 0)
    std::vector<Entity> GetEntitiesWith()
    {
        std::vector<Entity> entities;

        ForEach<T...>(
            [&](Entity entity, T&...)
            {
                entities.push_back(entity);
            }
        );

        return entities;
    }

    template<ComponentType... T, typename Fn>
        requires (sizeof...(T) > 0)
    void ForEach(Fn&& callback)
    {
        const void* typeIDs[] = { GetComponentTypeID<T>()... };
        constexpr usize componentCount = sizeof...(T);

        const void* smallestTypeID = typeIDs[0];
        usize smallestSize = GetComponentPoolSize(smallestTypeID);

        if (smallestSize == 0) return;

        for (usize i = 1; i < componentCount; ++i)
        {
            const usize poolSize = GetComponentPoolSize(typeIDs[i]);

            if (poolSize == 0) return;

            if (poolSize < smallestSize)
            {
                smallestTypeID = typeIDs[i];
                smallestSize = poolSize;
            }
        }

        std::vector<uint32> entityIDs;
        entityIDs.reserve(smallestSize);

        for (usize i = 0; i < smallestSize; ++i) entityIDs.push_back(GetComponentPoolEntityAt(smallestTypeID, i));

        ComponentIterScope iterScope(*this, typeIDs, componentCount);

        for (uint32 entityID : entityIDs)
        {
            if ((HasComponent<T>(entityID) && ...)) callback(Entity(entityID, this), *GetComponent<T>(entityID)...);
        }
    }

private:
    class ComponentIterScope
    {
    public:
        ComponentIterScope(Scene& _scene, const void* const* _typeIDs, usize _count)
            : scene(_scene), typeIDs(_typeIDs), count(_count)
        {
            for (usize i = 0; i < count; ++i) scene.BeginComponentPoolIter(typeIDs[i]);
        }

        ComponentIterScope(const ComponentIterScope&) = delete;
        ComponentIterScope& operator=(const ComponentIterScope&) = delete;

        ~ComponentIterScope()
        {
            for (usize i = 0; i < count; ++i) scene.EndComponentPoolIter(typeIDs[i]);
        }

    private:
        Scene& scene;
        const void* const* typeIDs;
        usize count;
    };

    class SceneImpl;
    std::unique_ptr<SceneImpl> _impl;

    friend class Entity;

    using MoveConstructFn = void(*)(void* dest, void* src);
    using DestroyFn = void(*)(void* object);

    uint32 nextEntityID = 1;
    uint32 activeCameraEntityID = MX_INVALID_ENTITY;

    void* AddComponent(uint32 entity, const void* typeID, usize size, usize align, MoveConstructFn moveConstruct, DestroyFn destroy, void* component);
    void* GetComponent(uint32 entity, const void* typeID);
    bool HasComponent(uint32 entity, const void* typeID) const;
    bool RemoveComponent(uint32 entity, const void* typeID);

    usize GetComponentPoolSize(const void* typeID) const;
    uint32 GetComponentPoolEntityAt(const void* typeID, usize index) const;

    void BeginComponentPoolIter(const void* typeID);
    void EndComponentPoolIter(const void* typeID);

    bool SetParent(Entity child, Entity parent);
    Entity GetParent(Entity entity);
    bool ClearParent(Entity child);

    std::vector<Entity> GetChildren(Entity entity);

    template<typename T>
    static const void* GetComponentTypeID()
    {
        static const char typeID = 0;

        return &typeID;
    }

    template<ComponentType T, typename... Args>
    T& AddComponent(uint32 entity, Args&&... args)
    {
        T component(std::forward<Args>(args)...);

        void* result = AddComponent(entity, GetComponentTypeID<T>(), sizeof(T), alignof(T),
            [](void* dest, void* src)
            {
                new (dest) T(std::move(*static_cast<T*>(src)));
            },
            [](void* object)
            {
                static_cast<T*>(object)->~T();
            }, &component);

        return *static_cast<T*>(result);
    }

    template<ComponentType T>
    T* GetComponent(uint32 entity) { return static_cast<T*>(GetComponent(entity, GetComponentTypeID<T>())); }

    template<ComponentType T>
    bool HasComponent(uint32 entity) const { return HasComponent(entity, GetComponentTypeID<T>()); }

    template<ComponentType T>
    bool RemoveComponent(uint32 entity) { return RemoveComponent(entity, GetComponentTypeID<T>()); }
};

template<ComponentType T, typename... Args>
T& Entity::AddComponent(Args&&... args)
{
    MX_CHECK(scene && id != MX_INVALID_ENTITY, "Cannot add component to invalid Entity");
    MX_CHECK(!scene->HasComponent<T>(id), "Entity already has component of tihs type");

    return scene->AddComponent<T>(id, std::forward<Args>(args)...);
}

template<ComponentType T, typename... Args>
T& Entity::GetOrAddComponent(Args&&... args)
{
    MX_CHECK(scene && id != MX_INVALID_ENTITY, "Cannot add component to invalid Entity");

    if (T* component = scene->GetComponent<T>(id)) return *component;

    return scene->AddComponent<T>(id, std::forward<Args>(args)...);
}

template<ComponentType T>
T* Entity::GetComponent()
{
    return scene ? scene->GetComponent<T>(id) : nullptr;
}

template<ComponentType T>
bool Entity::HasComponent() const
{
    return scene && scene->HasComponent<T>(id);
}

template<ComponentType T>
bool Entity::RemoveComponent()
{
    return scene && scene->RemoveComponent<T>(id);
}
