// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Scripting/ScriptSystem.h>
#include <MX/EngineContext.h>
#include <MX/Scene/Scene.h>
#include <MX/Scene/Component/ScriptComponent.h>

void ScriptSystem::Update(Scene& scene, EngineContext& engineContext, float64 deltaTime)
{
    scene.ForEach<ScriptComponent>(
        [&](Entity entity, ScriptComponent& scriptComponent)
        {
            for (std::unique_ptr<Script>& script : scriptComponent.scripts)
            {
                if (!script) continue;

                script->entity = entity;
                script->engineContext = &engineContext;

                if (!script->started)
                {
                    script->OnStart();
                    script->started = true;
                }

                script->OnUpdate(deltaTime);
            }
        }
    );
}
