// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Scripting/ScriptSystem.h>
#include <MX/EngineContext.h>
#include <MX/Scene/Scene.h>
#include <MX/Scene/Component/ScriptComponent.h>
#include <vector>

void ScriptSystem::Update(Scene& scene, EngineContext& engineContext, float64 deltaTime)
{
    std::vector<Entity> entities;

    scene.ForEach<ScriptComponent>(
        [&](Entity entity, ScriptComponent&)
        {
            entities.push_back(entity);
        }
    );

    for (Entity entity : entities)
    {
        ScriptComponent* scriptComponent = entity.GetComponent<ScriptComponent>();
        if (!scriptComponent) continue;

        const usize scriptCount = scriptComponent->scripts.size();

        for (usize i = 0; i < scriptCount; ++i)
        {
            ScriptComponent* currentComponent = entity.GetComponent<ScriptComponent>();
            if (!currentComponent || i >= currentComponent->scripts.size()) break;

            std::unique_ptr<Script> script = std::move(currentComponent->scripts[i]);
            if (!script) continue;

            script->entity = entity;
            script->engineContext = &engineContext;

            if (!script->started)
            {
                script->OnStart();
                script->started = true;

                currentComponent = entity.GetComponent<ScriptComponent>();
                if (!currentComponent || i >= currentComponent->scripts.size() || currentComponent->scripts[i]) continue;
            }

            script->OnUpdate(deltaTime);

            currentComponent = entity.GetComponent<ScriptComponent>();
            if (!currentComponent || i >= currentComponent->scripts.size() || currentComponent->scripts[i]) continue;

            currentComponent->scripts[i] = std::move(script);
        }
    }
}
