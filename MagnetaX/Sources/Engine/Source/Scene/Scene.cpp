// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Scene/Scene.h>
#include <MX/Scene/Component/Components.h>
#include "ComponentPool.h"
#include <unordered_map>
#include <algorithm>

class Scene::SceneImpl
{
public:
    SceneEnvironment environment;

    ComponentPool* FindComponentPool(const void* typeID)
    {
        const auto it = componentPools.find(typeID);

        if (it == componentPools.end()) return nullptr;

        return it->second.get();
    }

    ComponentPool& GetOrCreateComponentPool(const void* typeID, usize componentSize, usize componentAlign,
        ComponentPool::MoveConstructFn moveConstruct, ComponentPool::DestroyFn destroy)
    {
        if (ComponentPool* pool = FindComponentPool(typeID)) return *pool;

        std::unique_ptr<ComponentPool> pool = std::make_unique<ComponentPool>(componentSize, componentAlign, moveConstruct, destroy);
        ComponentPool* result = pool.get();

        componentPools.emplace(typeID, std::move(pool));

        return *result;
    }

    bool HasComponent(uint32 entity, const void* typeID) const
    {
        auto it = componentPools.find(typeID);
        if (it == componentPools.end()) return false;

        return it->second->Has(entity);
    }

    usize GetComponentPoolSize(const void* typeID) const
    {
        auto it = componentPools.find(typeID);

        if (it == componentPools.end()) return 0;

        return it->second->Size();
    }

    uint32 GetComponentPoolEntityAt(const void* typeID, usize index) const
    {
        auto it = componentPools.find(typeID);

        if (it == componentPools.end()) return MX_INVALID_ENTITY;

        return it->second->EntityAt(index);
    }

private:
    std::unordered_map<const void*, std::unique_ptr<ComponentPool>> componentPools;
};

Scene::Scene() : _impl(std::make_unique<SceneImpl>()) {}

Scene::~Scene() = default;

SceneEnvironment& Scene::GetEnvironment()
{
    return _impl->environment;
}

const SceneEnvironment& Scene::GetEnvironment() const
{
    return _impl->environment;
}

Entity Scene::CreateEntity()
{
    Entity entity(nextEntityID++, this);
    entity.AddComponent<TransformComponent>();

    return entity;
}

bool Scene::SetActiveCamera(Entity entity)
{
    if (entity.scene != this) return false;
    if (!entity.HasComponent<CameraComponent>()) return false;

    activeCameraEntityID = entity.id;

    return true;
}

void Scene::ClearActiveCamera()
{
    activeCameraEntityID = MX_INVALID_ENTITY;
}

Entity Scene::GetActiveCamera()
{
    if (activeCameraEntityID == MX_INVALID_ENTITY) return {};

    return Entity(activeCameraEntityID, this);
}

Entity Scene::GetEntityByID(uint32 id)
{
    if (id == MX_INVALID_ENTITY) return {};
    if (!HasComponent<TransformComponent>(id)) return {}; // Temporary check if Entity is proper

    return Entity(id, this);
}

std::vector<Entity> Scene::GetEntitiesByName(std::string_view name)
{
    std::vector<Entity> entities;

    ForEach<NameComponent>(
        [&](Entity entity, NameComponent& nameComponent)
        {
            if (nameComponent.name == name) entities.push_back(entity);
        }
    );

    return entities;
}

Matrix4f Scene::GetWorldMatrix(Entity entity) const
{
    if (entity.scene != this) return Matrix4f::Identity();

    TransformComponent* transformComponent = entity.GetComponent<TransformComponent>();
    if (!transformComponent) return Matrix4f::Identity();

    Matrix4f worldMatrix = Matrix4f::FromTranslationRotationScale(transformComponent->position,
        transformComponent->rotation, transformComponent->scale);

    Entity parent = transformComponent->parent;

    while (parent.id != MX_INVALID_ENTITY)
    {
        TransformComponent* parentTransform = parent.GetComponent<TransformComponent>();

        if (!parentTransform) break;

        const Matrix4f parentLocal = Matrix4f::FromTranslationRotationScale(parentTransform->position,
            parentTransform->rotation, parentTransform->scale);

        worldMatrix = parentLocal * worldMatrix;

        parent = parentTransform->parent;
    }

    return worldMatrix;
}

void* Scene::AddComponent(uint32 entity, const void* typeID, usize size, usize align,
    MoveConstructFn moveConstruct, DestroyFn destroy, void* component)
{
    ComponentPool& pool = _impl->GetOrCreateComponentPool(typeID, size, align, moveConstruct, destroy);

    if (void* result = pool.Insert(entity, component)) return result;

    return pool.Get(entity);
}

void* Scene::GetComponent(uint32 entity, const void* typeID)
{
    ComponentPool* pool = _impl->FindComponentPool(typeID);

    if (!pool) return nullptr;

    return pool->Get(entity);
}

bool Scene::HasComponent(uint32 entity, const void* typeID) const
{
    return _impl->HasComponent(entity, typeID);
}

bool Scene::RemoveComponent(uint32 entity, const void* typeID)
{
    if (typeID == GetComponentTypeID<TransformComponent>()) return false;

    ComponentPool* pool = _impl->FindComponentPool(typeID);

    if (!pool) return false;
    if (!pool->Remove(entity)) return false;

    if (typeID == GetComponentTypeID<CameraComponent>() && activeCameraEntityID == entity) activeCameraEntityID = MX_INVALID_ENTITY;

    return true;
}

usize Scene::GetComponentPoolSize(const void* typeID) const
{
    return _impl->GetComponentPoolSize(typeID);
}

uint32 Scene::GetComponentPoolEntityAt(const void* typeID, usize index) const
{
    return _impl->GetComponentPoolEntityAt(typeID, index);
}

void Scene::BeginComponentPoolIter(const void* typeID)
{
    ComponentPool* pool = _impl->FindComponentPool(typeID);

    MX_CHECK(pool, "Cannot begin iteration for missing ComponentPool");

    pool->BeginIter();
}

void Scene::EndComponentPoolIter(const void* typeID)
{
    ComponentPool* pool = _impl->FindComponentPool(typeID);

    MX_CHECK(pool, "Cannot end iteration for missing ComponentPool");

    pool->EndIter();
}

bool Scene::SetParent(Entity child, Entity parent)
{
    if (child.scene != this || parent.scene != this) return false;
    if (child.id == parent.id) return false;

    TransformComponent* childTransform = child.GetComponent<TransformComponent>();
    TransformComponent* parentTransform = parent.GetComponent<TransformComponent>();

    if (!childTransform || !parentTransform) return false;

    if (childTransform->parent.id == parent.id) return true;

    Entity current = parent;

    while (current.id != MX_INVALID_ENTITY)
    {
        if (current.id == child.id) return false;

        TransformComponent* currentTransform = current.GetComponent<TransformComponent>();

        if (!currentTransform) return false;

        current = currentTransform->parent;
    }

    if (childTransform->parent.id != MX_INVALID_ENTITY)
    {
        TransformComponent* oldParentTransform = childTransform->parent.GetComponent<TransformComponent>();

        if (oldParentTransform)
        {
            std::erase_if(oldParentTransform->children,
                [&](const Entity& entity)
                {
                    return entity.id == child.id;
                });
        }
    }

    childTransform->parent = parent;
    parentTransform->children.push_back(child);

    return true;
}

Entity Scene::GetParent(Entity entity)
{
    if (entity.scene != this) return {};

    TransformComponent* transform = entity.GetComponent<TransformComponent>();
    if (!transform) return {};

    return transform->parent;
}

bool Scene::ClearParent(Entity child)
{
    if (child.scene != this) return false;

    TransformComponent* childTransform = child.GetComponent<TransformComponent>();

    if (!childTransform) return false;
    if (childTransform->parent.id == MX_INVALID_ENTITY) return false;

    TransformComponent* parentTransform = childTransform->parent.GetComponent<TransformComponent>();

    if (parentTransform)
    {
        std::erase_if(parentTransform->children,
            [&](const Entity& entity)
            {
                return entity.id == child.id;
            });
    }

    childTransform->parent = {};

    return true;
}

std::vector<Entity> Scene::GetChildren(Entity entity)
{
    if (entity.scene != this) return {};

    TransformComponent* transform = entity.GetComponent<TransformComponent>();
    if (!transform) return {};

    return transform->children;
}
