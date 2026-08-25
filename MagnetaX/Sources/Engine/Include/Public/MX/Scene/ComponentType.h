// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <type_traits>

template<typename T>
concept ComponentType =
std::is_class_v<T> &&
std::is_default_constructible_v<T> &&
std::is_nothrow_move_constructible_v<T>;
