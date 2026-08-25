// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "WinAPIInput.h"

#if MX_PLATFORM_WINDOWS
#include <windowsx.h>

namespace
{
    KeyboardKeys TranslateKey(WPARAM vk, LPARAM lParam)
    {
        // Letters
        if (vk >= 'A' && vk <= 'Z') return (KeyboardKeys)((uint16)KeyboardKeys::A + (uint16)(vk - 'A'));

        // Digits
        if (vk >= '0' && vk <= '9') return (KeyboardKeys)((uint16)KeyboardKeys::DIGIT_0 + (uint16)(vk - '0'));

        // Function
        if (vk >= VK_F1 && vk <= VK_F12) return (KeyboardKeys)((uint16)KeyboardKeys::F1 + (uint16)(vk - VK_F1));

        // Numpad
        if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9) return (KeyboardKeys)((uint16)KeyboardKeys::NUMPAD_0 + (uint16)(vk - VK_NUMPAD0));

        switch (vk)
        {
        // Navigation
        case VK_UP:    return KeyboardKeys::ARROW_UP;
        case VK_DOWN:  return KeyboardKeys::ARROW_DOWN;
        case VK_LEFT:  return KeyboardKeys::ARROW_LEFT;
        case VK_RIGHT: return KeyboardKeys::ARROW_RIGHT;

        case VK_HOME:   return KeyboardKeys::HOME;
        case VK_END:    return KeyboardKeys::END;
        case VK_PRIOR:  return KeyboardKeys::PAGE_UP;
        case VK_NEXT:   return KeyboardKeys::PAGE_DOWN;
        case VK_INSERT: return KeyboardKeys::INSERT;
        case VK_DELETE: return KeyboardKeys::DEL;

        // Basic control
        case VK_SPACE:  return KeyboardKeys::SPACE;
        case VK_TAB:    return KeyboardKeys::TAB;
        case VK_BACK:   return KeyboardKeys::BACKSPACE;
        case VK_ESCAPE: return KeyboardKeys::ESCAPE;

        case VK_RETURN:
        {
            const bool extended = (lParam & 0x01000000) != 0;
            return extended ? KeyboardKeys::NUMPAD_ENTER : KeyboardKeys::ENTER;
        }

        // Modifiers
        case VK_SHIFT:
        {
            const UINT scanCode = (UINT)((lParam & 0x00FF0000) >> 16);
            const UINT extendedVk = MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX);

            return extendedVk == VK_RSHIFT ? KeyboardKeys::RIGHT_SHIFT : KeyboardKeys::LEFT_SHIFT;
        }

        case VK_CONTROL:
        {
            const bool extended = (lParam & 0x01000000) != 0;
            return extended ? KeyboardKeys::RIGHT_CTRL : KeyboardKeys::LEFT_CTRL;
        }

        case VK_MENU:
        {
            const bool extended = (lParam & 0x01000000) != 0;
            return extended ? KeyboardKeys::RIGHT_ALT : KeyboardKeys::LEFT_ALT;
        }

        case VK_LWIN: return KeyboardKeys::LEFT_SUPER;
        case VK_RWIN: return KeyboardKeys::RIGHT_SUPER;

        // Locks / system
        case VK_CAPITAL:  return KeyboardKeys::CAPS_LOCK;
        case VK_NUMLOCK:  return KeyboardKeys::NUM_LOCK;
        case VK_SCROLL:   return KeyboardKeys::SCROLL_LOCK;
        case VK_SNAPSHOT: return KeyboardKeys::PRINT_SCREEN;
        case VK_PAUSE:    return KeyboardKeys::PAUSE;

        // Punctuation
        case VK_OEM_3:      return KeyboardKeys::GRAVE;
        case VK_OEM_MINUS:  return KeyboardKeys::MINUS;
        case VK_OEM_PLUS:   return KeyboardKeys::EQUAL;
        case VK_OEM_4:      return KeyboardKeys::LEFT_BRACKET;
        case VK_OEM_6:      return KeyboardKeys::RIGHT_BRACKET;
        case VK_OEM_5:      return KeyboardKeys::BACKSLASH;
        case VK_OEM_1:      return KeyboardKeys::SEMICOLON;
        case VK_OEM_7:      return KeyboardKeys::APOSTROPHE;
        case VK_OEM_COMMA:  return KeyboardKeys::COMMA;
        case VK_OEM_PERIOD: return KeyboardKeys::PERIOD;
        case VK_OEM_2:      return KeyboardKeys::SLASH;

        // Numpad
        case VK_DECIMAL:  return KeyboardKeys::NUMPAD_DECIMAL;
        case VK_DIVIDE:   return KeyboardKeys::NUMPAD_DIVIDE;
        case VK_MULTIPLY: return KeyboardKeys::NUMPAD_MULTIPLY;
        case VK_SUBTRACT: return KeyboardKeys::NUMPAD_SUBTRACT;
        case VK_ADD:      return KeyboardKeys::NUMPAD_ADD;
        }

        return KeyboardKeys::UNKNOWN;
    }
}

bool WinAPIInput::ProcessMessage(InputFeed& inputFeed, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        const KeyboardKeys key = TranslateKey(wParam, lParam);

        if (key != KeyboardKeys::UNKNOWN) inputFeed.SetKeyDown(key, true);

        return true;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        const KeyboardKeys key = TranslateKey(wParam, lParam);

        if (key != KeyboardKeys::UNKNOWN) inputFeed.SetKeyDown(key, false);

        return true;
    }
    case WM_LBUTTONDOWN:
    {
        inputFeed.SetMouseButton(MouseButtons::LEFT, true);

        return true;
    }
    case WM_LBUTTONUP:
    {
        inputFeed.SetMouseButton(MouseButtons::LEFT, false);

        return true;
    }
    case WM_RBUTTONDOWN:
    {
        inputFeed.SetMouseButton(MouseButtons::RIGHT, true);

        return true;
    }
    case WM_RBUTTONUP:
    {
        inputFeed.SetMouseButton(MouseButtons::RIGHT, false);

        return true;
    }
    case WM_MBUTTONDOWN:
    {
        inputFeed.SetMouseButton(MouseButtons::MIDDLE, true);

        return true;
    }
    case WM_MBUTTONUP:
    {
        inputFeed.SetMouseButton(MouseButtons::MIDDLE, false);

        return true;
    }
    case WM_XBUTTONDOWN:
    {
        const WORD xbtn = GET_XBUTTON_WPARAM(wParam);

        inputFeed.SetMouseButton(xbtn == XBUTTON1 ? MouseButtons::X1 : MouseButtons::X2, true);

        return true;
    }
    case WM_XBUTTONUP:
    {
        const WORD xbtn = GET_XBUTTON_WPARAM(wParam);

        inputFeed.SetMouseButton(xbtn == XBUTTON1 ? MouseButtons::X1 : MouseButtons::X2, false);

        return true;
    }
    case WM_MOUSEMOVE:
    {
        const int32 x = GET_X_LPARAM(lParam);
        const int32 y = GET_Y_LPARAM(lParam);

        inputFeed.SetMousePosition(x, y);

        return true;
    }
    case WM_MOUSEWHEEL:
    {
        const int32 delta = GET_WHEEL_DELTA_WPARAM(wParam);

        inputFeed.AddMouseWheel(delta);

        return true;
    }
    }

    return false;
}
#endif
