// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "../CoreMinimal.h"
#include <memory>

class Clock
{
public:
    Clock();
    Clock(Clock&&) noexcept;
    Clock& operator=(Clock&&) noexcept;

    Clock(const Clock&) = delete;
    Clock& operator=(const Clock&) = delete;

    ~Clock();

    float64 Now() const;
    float64 Delta() const;

    void Update();
    void Reset();

private:
    struct ClockImpl;
    std::unique_ptr<ClockImpl> _impl;
};
