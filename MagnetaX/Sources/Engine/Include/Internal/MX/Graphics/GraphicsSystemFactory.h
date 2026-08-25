// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <memory>

class AbstractGraphicsSystem;

std::unique_ptr<AbstractGraphicsSystem> CreateGraphicsSystem();
