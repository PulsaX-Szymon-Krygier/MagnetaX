// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "ImGuiEditorUI.h"
#include <MX/Scene/Scene.h>
#include <MX/Scene/Component/NameComponent.h>
#include <MX/Scene/Component/TransformComponent.h>
#include <EditorContext.h>
#include <imgui.h>

void ImGuiEditorUI::Draw(EditorContext& context)
{
    ImGui::DockSpaceOverViewport();

    ImGui::Begin("Hierarchy");

    if (context.scene)
    {
        context.scene->ForEach<TransformComponent>(
            [this, &context](Entity entity, TransformComponent&)
            {
                if (entity.GetParent().GetID() != MX_INVALID_ENTITY) return;

                DrawEntityNode(entity, context);
            });
    }

    ImGui::End();

    ImGui::Begin("Inspector");

    if (context.selectedEntity.GetID() != MX_INVALID_ENTITY)
    {
        Entity entity = context.selectedEntity;

        if (NameComponent* name = entity.GetComponent<NameComponent>())
        {
            ImGui::Text("Name: %s", name->name.c_str());
        }
        else
        {
            ImGui::Text("Entity %u", entity.GetID());
        }

        if (TransformComponent* transform = entity.GetComponent<TransformComponent>())
        {
            ImGui::SeparatorText("Transform");

            float32 position[] =
            {
                transform->position.x,
                transform->position.y,
                transform->position.z
            };

            if (ImGui::DragFloat3("Position", position, 0.1f))
            {
                transform->position.Set(position[0], position[1], position[2]);
            }

            Vector3f& rotationEuler = context.rotationEulerHints.try_emplace(entity.GetID(), Vector3f(0.0f)).first->second;

            float32 rotation[] =
            {
                rotationEuler.x,
                rotationEuler.y,
                rotationEuler.z
            };

            if (ImGui::DragFloat3("Rotation", rotation, 1.0f))
            {
                rotationEuler.Set(rotation[0], rotation[1], rotation[2]);
                transform->rotation = Quaternion::FromYawPitchRollDegrees(rotationEuler.y, rotationEuler.x, rotationEuler.z);
            }

            float32 scale[] =
            {
                transform->scale.x,
                transform->scale.y,
                transform->scale.z
            };

            if (ImGui::DragFloat3("Scale", scale, 0.1f))
            {
                transform->scale.Set(scale[0], scale[1], scale[2]);
            }
        }
    }

    ImGui::End();
}

void ImGuiEditorUI::DrawEntityNode(Entity entity, EditorContext& context)
{
    NameComponent* name = entity.GetComponent<NameComponent>();
    const auto children = entity.GetChildren();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

    if (context.selectedEntity.GetID() == entity.GetID()) flags |= ImGuiTreeNodeFlags_Selected;
    if (children.empty()) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    ImGui::PushID((int)entity.GetID());

    const bool open = name ? ImGui::TreeNodeEx(name->name.c_str(), flags) : ImGui::TreeNodeEx("Entity", flags, "Entity %u", entity.GetID());

    if (ImGui::IsItemClicked()) context.selectedEntity = entity;

    if (open && !children.empty())
    {
        for (Entity child : children)
        {
            DrawEntityNode(child, context);
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
}
