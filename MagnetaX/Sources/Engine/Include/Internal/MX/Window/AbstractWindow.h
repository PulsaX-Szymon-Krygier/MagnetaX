// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Window/IWindow.h>
#include <MX/Input/InputFeed.h>
#include "SurfaceHost.h"
#include <vector>

class AbstractWindow : public IWindow, public SurfaceHost
{
public:
    virtual ~AbstractWindow() = default;

    /* IWindow */
    Size2i GetSize() const override { return config.size; }
    std::string_view GetTitle() const override { return config.title; }
    WindowState GetState() const override { return config.state; }
    bool GetVisibility() const override { return config.visible; }
    bool GetResizable() const override { return config.resizable; }

    uint32 Subscribe(WindowEventCallback callback, void* user) override
    {
        Subscriber subscriber{};
        subscriber.token = ++nextToken;
        subscriber.callback = callback;
        subscriber.user = user;
        subscribers.push_back(subscriber);

        return subscriber.token;
    }

    void Unsubscribe(uint32 token) override
    {
        for (usize i = 0; i < subscribers.size(); ++i)
        {
            if (subscribers[i].token == token)
            {
                subscribers.erase(subscribers.begin() + (isize)i);

                return;
            }
        }
    }

    /* AbstractWindow */
    virtual bool Create(const WindowConfig& createInfo = WindowConfig()) = 0;
    virtual void Destroy() = 0;

    void SetConfiguration(const WindowConfig& newConfig)
    {
        // Order is fixed, do not change it until u know what are u doing!
        SetResizable(newConfig.resizable);
        SetSize(newConfig.size);
        SetTitle(newConfig.title);
        SetState(newConfig.state);
        SetVisibility(newConfig.visible);
    }

    virtual void PollEvents() = 0;
    virtual void SetInputFeed(InputFeed* feed) { inputFeed = feed; }

    /* SurfaceHost */
    Size2i GetSurfaceSize() const override { return GetSize(); }

protected:
    WindowConfig config;

    struct Subscriber
    {
        uint32 token = 0;
        WindowEventCallback callback = nullptr;
        void* user = nullptr;
    };

    uint32 nextToken = 0;
    std::vector<Subscriber> subscribers;

    InputFeed* inputFeed = nullptr;

    void DispatchEvent(WindowEventType type)
    {
        if (type == WindowEventType::FOCUS_LOST && inputFeed) inputFeed->Reset();

        WindowEvent event{};
        event.eventType = type;
        event.windowConfig = config;

        std::vector<uint32> tokens;
        tokens.reserve(subscribers.size());

        for (const Subscriber& subscriber : subscribers) tokens.push_back(subscriber.token);

        for (uint32 token : tokens)
        {
            for (const Subscriber& subscriber : subscribers)
            {
                if (subscriber.token != token) continue;

                const WindowEventCallback callback = subscriber.callback;
                void* user = subscriber.user;

                if (callback) callback(event, user);

                break;
            }
        }
    }
};
