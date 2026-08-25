// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "X11Input.h"

#if MX_PLATFORM_X11
#include <X11/XKBlib.h>
#include <X11/keysym.h>

namespace
{
    constexpr uint32 X11_BUTTON_X1 = 8;
    constexpr uint32 X11_BUTTON_X2 = 9;
    constexpr int32 X11_WHEEL_DELTA = 120;

    KeyboardKeys TranslateKey(XKeyEvent& event)
    {
        const KeySym keySym = XLookupKeysym(&event, 0);

        // Letters
        if (keySym >= XK_a && keySym <= XK_z) return (KeyboardKeys)((uint16)KeyboardKeys::A + (uint16)(keySym - XK_a));
        if (keySym >= XK_A && keySym <= XK_Z) return (KeyboardKeys)((uint16)KeyboardKeys::A + (uint16)(keySym - XK_A));

        // Digits
        if (keySym >= XK_0 && keySym <= XK_9) return (KeyboardKeys)((uint16)KeyboardKeys::DIGIT_0 + (uint16)(keySym - XK_0));

        // Function
        if (keySym >= XK_F1 && keySym <= XK_F12) return (KeyboardKeys)((uint16)KeyboardKeys::F1 + (uint16)(keySym - XK_F1));

        // Numpad
        if (keySym >= XK_KP_0 && keySym <= XK_KP_9) return (KeyboardKeys)((uint16)KeyboardKeys::NUMPAD_0 + (uint16)(keySym - XK_KP_0));

        switch (keySym)
        {
        // Navigation
        case XK_Up:    return KeyboardKeys::ARROW_UP;
        case XK_Down:  return KeyboardKeys::ARROW_DOWN;
        case XK_Left:  return KeyboardKeys::ARROW_LEFT;
        case XK_Right: return KeyboardKeys::ARROW_RIGHT;

        case XK_Home:      return KeyboardKeys::HOME;
        case XK_End:       return KeyboardKeys::END;
        case XK_Page_Up:   return KeyboardKeys::PAGE_UP;
        case XK_Page_Down: return KeyboardKeys::PAGE_DOWN;
        case XK_Insert:    return KeyboardKeys::INSERT;
        case XK_Delete:    return KeyboardKeys::DEL;

        // Basic control
        case XK_space: return KeyboardKeys::SPACE;

        case XK_Tab:
        case XK_ISO_Left_Tab: return KeyboardKeys::TAB;

        case XK_Return:    return KeyboardKeys::ENTER;
        case XK_BackSpace: return KeyboardKeys::BACKSPACE;
        case XK_Escape:    return KeyboardKeys::ESCAPE;

        // Modifiers
        case XK_Shift_L: return KeyboardKeys::LEFT_SHIFT;
        case XK_Shift_R: return KeyboardKeys::RIGHT_SHIFT;

        case XK_Control_L: return KeyboardKeys::LEFT_CTRL;
        case XK_Control_R: return KeyboardKeys::RIGHT_CTRL;

        case XK_Alt_L: return KeyboardKeys::LEFT_ALT;

        case XK_Alt_R:
        case XK_ISO_Level3_Shift: return KeyboardKeys::RIGHT_ALT;

        case XK_Super_L: return KeyboardKeys::LEFT_SUPER;
        case XK_Super_R: return KeyboardKeys::RIGHT_SUPER;

        // Locks / system
        case XK_Caps_Lock:   return KeyboardKeys::CAPS_LOCK;
        case XK_Num_Lock:    return KeyboardKeys::NUM_LOCK;
        case XK_Scroll_Lock: return KeyboardKeys::SCROLL_LOCK;
        case XK_Print:       return KeyboardKeys::PRINT_SCREEN;
        case XK_Pause:       return KeyboardKeys::PAUSE;

        // Punctuation
        case XK_grave:        return KeyboardKeys::GRAVE;
        case XK_minus:        return KeyboardKeys::MINUS;
        case XK_equal:        return KeyboardKeys::EQUAL;
        case XK_bracketleft:  return KeyboardKeys::LEFT_BRACKET;
        case XK_bracketright: return KeyboardKeys::RIGHT_BRACKET;
        case XK_backslash:    return KeyboardKeys::BACKSLASH;
        case XK_semicolon:    return KeyboardKeys::SEMICOLON;
        case XK_apostrophe:   return KeyboardKeys::APOSTROPHE;
        case XK_comma:        return KeyboardKeys::COMMA;
        case XK_period:       return KeyboardKeys::PERIOD;
        case XK_slash:        return KeyboardKeys::SLASH;

        // Numpad
        case XK_KP_Decimal:
        case XK_KP_Delete: return KeyboardKeys::NUMPAD_DECIMAL;

        case XK_KP_Divide:   return KeyboardKeys::NUMPAD_DIVIDE;
        case XK_KP_Multiply: return KeyboardKeys::NUMPAD_MULTIPLY;
        case XK_KP_Subtract: return KeyboardKeys::NUMPAD_SUBTRACT;
        case XK_KP_Add:      return KeyboardKeys::NUMPAD_ADD;
        case XK_KP_Enter:    return KeyboardKeys::NUMPAD_ENTER;

        // Numpad (numlock off [but test it someday])
        case XK_KP_Insert:    return KeyboardKeys::NUMPAD_0;
        case XK_KP_End:       return KeyboardKeys::NUMPAD_1;
        case XK_KP_Down:      return KeyboardKeys::NUMPAD_2;
        case XK_KP_Page_Down: return KeyboardKeys::NUMPAD_3;
        case XK_KP_Left:      return KeyboardKeys::NUMPAD_4;
        case XK_KP_Begin:     return KeyboardKeys::NUMPAD_5;
        case XK_KP_Right:     return KeyboardKeys::NUMPAD_6;
        case XK_KP_Home:      return KeyboardKeys::NUMPAD_7;
        case XK_KP_Up:        return KeyboardKeys::NUMPAD_8;
        case XK_KP_Page_Up:   return KeyboardKeys::NUMPAD_9;
        }

        return KeyboardKeys::UNKNOWN;
    }
}

void X11Input::Initialize(Display* display)
{
    if (!display) return;

    XkbSetDetectableAutoRepeat(display, True, nullptr);
}

bool X11Input::ProcessEvent(InputFeed& inputFeed, XEvent& event)
{
    switch (event.type)
    {
    case MappingNotify:
    {
        if (event.xmapping.request == MappingKeyboard || event.xmapping.request == MappingModifier) XRefreshKeyboardMapping(&event.xmapping);

        return true;
    }
    case KeyPress:
    case KeyRelease:
    {
        const KeyboardKeys key = TranslateKey(event.xkey);

        if (key != KeyboardKeys::UNKNOWN) inputFeed.SetKeyDown(key, event.type == KeyPress);

        return true;
    }
    case ButtonPress:
    {
        switch (event.xbutton.button)
        {
        case Button1:
            inputFeed.SetMouseButton(MouseButtons::LEFT, true);
            break;

        case Button2:
            inputFeed.SetMouseButton(MouseButtons::MIDDLE, true);
            break;

        case Button3:
            inputFeed.SetMouseButton(MouseButtons::RIGHT, true);
            break;

        case Button4:
            inputFeed.AddMouseWheel(X11_WHEEL_DELTA);
            break;

        case Button5:
            inputFeed.AddMouseWheel(-X11_WHEEL_DELTA);
            break;

        case X11_BUTTON_X1:
            inputFeed.SetMouseButton(MouseButtons::X1, true);
            break;

        case X11_BUTTON_X2:
            inputFeed.SetMouseButton(MouseButtons::X2, true);
            break;

        default:
            return false;
        }

        return true;
    }
    case ButtonRelease:
    {
        switch (event.xbutton.button)
        {
        case Button1:
            inputFeed.SetMouseButton(MouseButtons::LEFT, false);
            break;

        case Button2:
            inputFeed.SetMouseButton(MouseButtons::MIDDLE, false);
            break;

        case Button3:
            inputFeed.SetMouseButton(MouseButtons::RIGHT, false);
            break;

        case Button4:
        case Button5:
            break;

        case X11_BUTTON_X1:
            inputFeed.SetMouseButton(MouseButtons::X1, false);
            break;

        case X11_BUTTON_X2:
            inputFeed.SetMouseButton(MouseButtons::X2, false);
            break;

        default:
            return false;
        }

        return true;
    }
    case MotionNotify:
    {
        inputFeed.SetMousePosition((int32)event.xmotion.x, (int32)event.xmotion.y);

        return true;
    }
    }

    return false;
}
#endif
