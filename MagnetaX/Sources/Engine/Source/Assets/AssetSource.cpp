// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Assets/AssetSource.h>
#include <utility>

AssetSource::AssetSource(std::string _path) : path(std::move(_path)) {}
