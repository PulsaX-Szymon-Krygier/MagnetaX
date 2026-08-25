// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Scripting/Script.h>
#include <concepts>
#include <memory>
#include <utility>
#include <vector>

class ScriptSystem;

class ScriptComponent
{
public:
    template<typename T, typename... Args>
    requires std::derived_from<T, Script>
    T& AddScript(Args&&... args)
    {
        std::unique_ptr<T> script = std::make_unique<T>(std::forward<Args>(args)...);

        T* result = script.get();

        scripts.push_back(std::move(script));

        return *result;
    }

private:
    friend class ScriptSystem;

    std::vector<std::unique_ptr<Script>> scripts;
};
