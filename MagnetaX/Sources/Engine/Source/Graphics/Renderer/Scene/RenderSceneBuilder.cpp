// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "RenderSceneBuilder.h"
#include <MX/Core/Math/MathConst.h>
#include <MX/Scene/Component/CameraComponent.h>
#include <MX/Scene/Component/LightComponent.h>
#include <MX/Scene/Component/MaterialComponent.h>
#include <MX/Scene/Component/MeshComponent.h>
#include <MX/Scene/Scene.h>

RenderSceneData BuildRenderSceneData(Scene* scene, const Size2i& renderSize)
{
    RenderSceneData sceneData{};

    if (!scene) return sceneData;

    const SceneEnvironment& environment = scene->GetEnvironment();

    sceneData.backgroundColor = environment.backgroundColor;
    sceneData.ambientLightColor = environment.ambientLightColor;
    sceneData.ambientLightIntensity = environment.ambientLightIntensity;
    sceneData.environmentMap = environment.environmentMap;

    scene->ForEach<LightComponent>(
        [&](Entity entity, LightComponent& lightComponent)
        {
            RenderLight light{};
            light.type = lightComponent.type;
            light.color = lightComponent.color;
            light.intensity = lightComponent.intensity;
            light.range = lightComponent.range;
            light.innerConeAngle = lightComponent.innerConeAngle;
            light.outerConeAngle = lightComponent.outerConeAngle;
            light.castsShadows = lightComponent.castShadows;

            const Matrix4f worldMatrix = entity.GetWorldMatrix();

            light.position = Vector3f(worldMatrix.m03, worldMatrix.m13, worldMatrix.m23);
            light.direction = Vector3f(-worldMatrix.m02, -worldMatrix.m12, -worldMatrix.m22);

            if (light.direction.LengthSquared() > MX_MATH_EPSILON_SQUARED) light.direction.Normalize();

            sceneData.lights.push_back(light);
        }
    );

    if (renderSize.width == 0 || renderSize.height == 0) return sceneData;

    Entity cameraEntity = scene->GetActiveCamera();
    if (cameraEntity.GetID() == MX_INVALID_ENTITY) return sceneData;

    CameraComponent* cameraComponent = cameraEntity.GetComponent<CameraComponent>();
    if (!cameraComponent) return sceneData;

    const float32 aspect = (float32)renderSize.width / (float32)renderSize.height;

    const Matrix4f cameraWorldMatrix = cameraEntity.GetWorldMatrix();
    const Matrix4f viewMatrix = cameraWorldMatrix.Inversed();
    const Matrix4f projectionMatrix = cameraComponent->GetProjectionMatrix(aspect);
    const Matrix4f viewProjMatrix = projectionMatrix * viewMatrix;

    sceneData.view = viewMatrix;
    sceneData.viewProjectionInversed = viewProjMatrix.Inversed();
    sceneData.cameraPosition = Vector3f(cameraWorldMatrix.m03, cameraWorldMatrix.m13, cameraWorldMatrix.m23);
    sceneData.cameraNearPlane = cameraComponent->nearPlane;
    sceneData.cameraFarPlane = cameraComponent->farPlane;
    sceneData.exposureEV = cameraComponent->exposureEV;

    scene->ForEach<MeshComponent>(
        [&](Entity entity, MeshComponent& meshComponent)
        {
            if (!meshComponent.mesh) return;

            RenderObject object{};
            object.mesh = meshComponent.mesh;
            object.model = entity.GetWorldMatrix();
            object.mvp = viewProjMatrix * object.model;

            MaterialComponent* materialComponent = entity.GetComponent<MaterialComponent>();

            if (materialComponent) object.material = materialComponent->material;

            sceneData.objects.push_back(object);
        }
    );

    return sceneData;
}
