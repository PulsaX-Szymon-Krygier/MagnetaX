// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Core/Time/Clock.h>
#include <chrono>

struct Clock::ClockImpl
{
    using SteadyClock = std::chrono::steady_clock;

    SteadyClock::time_point startTime;
    SteadyClock::time_point lastTime;
    SteadyClock::time_point currentTime;

    float64 delta = 0.0;

    ClockImpl() : startTime(SteadyClock::now()), lastTime(startTime), currentTime(startTime) {}

    float64 Now() const
    {
        return std::chrono::duration<float64>(currentTime - startTime).count();
    }

    void Update()
    {
        currentTime = SteadyClock::now();
        delta = std::chrono::duration<float64>(currentTime - lastTime).count();
        lastTime = currentTime;
    }

    void Reset()
    {
        const SteadyClock::time_point now = SteadyClock::now();

        startTime = now;
        lastTime = now;
        currentTime = now;
        delta = 0.0;
    }
};

Clock::Clock() : _impl(std::make_unique<ClockImpl>()) {}

Clock::Clock(Clock&&) noexcept = default;
Clock& Clock::operator=(Clock&&) noexcept = default;

Clock::~Clock() = default;

float64 Clock::Now() const
{
    return _impl->Now();
}

float64 Clock::Delta() const
{
    return _impl->delta;
}

void Clock::Update()
{
    _impl->Update();
}

void Clock::Reset()
{
    _impl->Reset();
}
