// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Window/AbstractWindow.h>
#include <memory>

std::unique_ptr<AbstractWindow> CreatePlatformWindow();
