// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "CameraScript.h"
#include <MX/Input/Input.h>
#include <MX/Scene/Component/TransformComponent.h>

void CameraScript::OnUpdate(float64 deltaTime)
{
    TransformComponent* transform = GetEntity().GetComponent<TransformComponent>();

    const Input::Keyboard keyboard = GetInput().GetKeyboard();
    const Input::Mouse mouse = GetInput().GetMouse();

    if (mouse.ButtonDown(MouseButtons::RIGHT) || mouse.ButtonDown(MouseButtons::LEFT))
    {
        yaw -= (float32)(mouse.GetDX() * mouseSens);
        pitch -= (float32)(mouse.GetDY() * mouseSens);

        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        transform->rotation = Quaternion::FromYawPitchRollDegrees(yaw, pitch, 0.0f);
    }

    const Vector3f forward = transform->rotation.RotateVector(Vector3f(0.0f, 0.0f, -1.0f));
    const Vector3f right = transform->rotation.RotateVector(Vector3f(1.0f, 0.0f, 0.0f));

    const float32 mov = moveSpeed * (float32)deltaTime * 2.0f;


    if (keyboard.KeyDown(KeyboardKeys::W)) transform->position += forward * mov;
    if (keyboard.KeyDown(KeyboardKeys::S)) transform->position -= forward * mov;
    if (keyboard.KeyDown(KeyboardKeys::A)) transform->position -= right * mov;
    if (keyboard.KeyDown(KeyboardKeys::D)) transform->position += right * mov;
    if (keyboard.KeyDown(KeyboardKeys::SPACE)) transform->position.y += mov;
    if (keyboard.KeyDown(KeyboardKeys::LEFT_SHIFT)) transform->position.y -= mov;
}
