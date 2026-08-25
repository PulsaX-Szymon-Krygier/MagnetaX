// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "CocoaInput.h"

#if MX_PLATFORM_APPLE
#import <Cocoa/Cocoa.h>
#include <cmath>

namespace
{
    constexpr int32 COCOA_WHEEL_DELTA = 120;

    // Translations are not fully tested yet
    // I am not sure if those are ok
    // TODO: check later
    // https://macbiblioblog.blogspot.com/2014/12/key-codes-for-function-and-special-keys.html
    KeyboardKeys TranslateKey(unsigned short keyCode)
    {
        switch (keyCode)
        {
        // Letters
        case 0:  return KeyboardKeys::A;
        case 11: return KeyboardKeys::B;
        case 8:  return KeyboardKeys::C;
        case 2:  return KeyboardKeys::D;
        case 14: return KeyboardKeys::E;
        case 3:  return KeyboardKeys::F;
        case 5:  return KeyboardKeys::G;
        case 4:  return KeyboardKeys::H;
        case 34: return KeyboardKeys::I;
        case 38: return KeyboardKeys::J;
        case 40: return KeyboardKeys::K;
        case 37: return KeyboardKeys::L;
        case 46: return KeyboardKeys::M;
        case 45: return KeyboardKeys::N;
        case 31: return KeyboardKeys::O;
        case 35: return KeyboardKeys::P;
        case 12: return KeyboardKeys::Q;
        case 15: return KeyboardKeys::R;
        case 1:  return KeyboardKeys::S;
        case 17: return KeyboardKeys::T;
        case 32: return KeyboardKeys::U;
        case 9:  return KeyboardKeys::V;
        case 13: return KeyboardKeys::W;
        case 7:  return KeyboardKeys::X;
        case 16: return KeyboardKeys::Y;
        case 6:  return KeyboardKeys::Z;

        // Digits
        case 29: return KeyboardKeys::DIGIT_0;
        case 18: return KeyboardKeys::DIGIT_1;
        case 19: return KeyboardKeys::DIGIT_2;
        case 20: return KeyboardKeys::DIGIT_3;
        case 21: return KeyboardKeys::DIGIT_4;
        case 23: return KeyboardKeys::DIGIT_5;
        case 22: return KeyboardKeys::DIGIT_6;
        case 26: return KeyboardKeys::DIGIT_7;
        case 28: return KeyboardKeys::DIGIT_8;
        case 25: return KeyboardKeys::DIGIT_9;

        // Function
        case 122: return KeyboardKeys::F1;
        case 120: return KeyboardKeys::F2;
        case 99:  return KeyboardKeys::F3;
        case 118: return KeyboardKeys::F4;
        case 96:  return KeyboardKeys::F5;
        case 97:  return KeyboardKeys::F6;
        case 98:  return KeyboardKeys::F7;
        case 100: return KeyboardKeys::F8;
        case 101: return KeyboardKeys::F9;
        case 109: return KeyboardKeys::F10;
        case 103: return KeyboardKeys::F11;
        case 111: return KeyboardKeys::F12;

        // Navigation
        case 126: return KeyboardKeys::ARROW_UP;
        case 125: return KeyboardKeys::ARROW_DOWN;
        case 123: return KeyboardKeys::ARROW_LEFT;
        case 124: return KeyboardKeys::ARROW_RIGHT;

        case 115: return KeyboardKeys::HOME;
        case 119: return KeyboardKeys::END;
        case 116: return KeyboardKeys::PAGE_UP;
        case 121: return KeyboardKeys::PAGE_DOWN;
        case 114: return KeyboardKeys::INSERT;
        case 117: return KeyboardKeys::DEL;

        // Basic control
        case 49: return KeyboardKeys::SPACE;
        case 48: return KeyboardKeys::TAB;
        case 36: return KeyboardKeys::ENTER;
        case 51: return KeyboardKeys::BACKSPACE;
        case 53: return KeyboardKeys::ESCAPE;

        // Modifiers
        case 56: return KeyboardKeys::LEFT_SHIFT;
        case 60: return KeyboardKeys::RIGHT_SHIFT;
        case 59: return KeyboardKeys::LEFT_CTRL;
        case 62: return KeyboardKeys::RIGHT_CTRL;
        case 58: return KeyboardKeys::LEFT_ALT;
        case 61: return KeyboardKeys::RIGHT_ALT;
        case 55: return KeyboardKeys::LEFT_SUPER;
        case 54: return KeyboardKeys::RIGHT_SUPER;

        case 57: return KeyboardKeys::CAPS_LOCK;

        // Punctuation
        case 50: return KeyboardKeys::GRAVE;
        case 27: return KeyboardKeys::MINUS;
        case 24: return KeyboardKeys::EQUAL;
        case 33: return KeyboardKeys::LEFT_BRACKET;
        case 30: return KeyboardKeys::RIGHT_BRACKET;
        case 42: return KeyboardKeys::BACKSLASH;
        case 41: return KeyboardKeys::SEMICOLON;
        case 39: return KeyboardKeys::APOSTROPHE;
        case 43: return KeyboardKeys::COMMA;
        case 47: return KeyboardKeys::PERIOD;
        case 44: return KeyboardKeys::SLASH;

        // Numpad
        case 82: return KeyboardKeys::NUMPAD_0;
        case 83: return KeyboardKeys::NUMPAD_1;
        case 84: return KeyboardKeys::NUMPAD_2;
        case 85: return KeyboardKeys::NUMPAD_3;
        case 86: return KeyboardKeys::NUMPAD_4;
        case 87: return KeyboardKeys::NUMPAD_5;
        case 88: return KeyboardKeys::NUMPAD_6;
        case 89: return KeyboardKeys::NUMPAD_7;
        case 91: return KeyboardKeys::NUMPAD_8;
        case 92: return KeyboardKeys::NUMPAD_9;

        case 65: return KeyboardKeys::NUMPAD_DECIMAL;
        case 75: return KeyboardKeys::NUMPAD_DIVIDE;
        case 67: return KeyboardKeys::NUMPAD_MULTIPLY;
        case 78: return KeyboardKeys::NUMPAD_SUBTRACT;
        case 69: return KeyboardKeys::NUMPAD_ADD;
        case 76: return KeyboardKeys::NUMPAD_ENTER;
        }

        return KeyboardKeys::UNKNOWN;
    }

    NSEventModifierFlags GetModifierFlag(KeyboardKeys key)
    {
        switch (key)
        {
        case KeyboardKeys::LEFT_SHIFT:
        case KeyboardKeys::RIGHT_SHIFT:
            return NSEventModifierFlagShift;

        case KeyboardKeys::LEFT_CTRL:
        case KeyboardKeys::RIGHT_CTRL:
            return NSEventModifierFlagControl;

        case KeyboardKeys::LEFT_ALT:
        case KeyboardKeys::RIGHT_ALT:
            return NSEventModifierFlagOption;

        case KeyboardKeys::LEFT_SUPER:
        case KeyboardKeys::RIGHT_SUPER:
            return NSEventModifierFlagCommand;

        case KeyboardKeys::CAPS_LOCK:
            return NSEventModifierFlagCapsLock;

        default:
            return 0;
        }
    }
}

bool CocoaInput::ProcessEvent(InputFeed& inputFeed, NSEvent* event, NSView* view)
{
    if (!event) return false;

    switch (event.type)
    {
    case NSEventTypeKeyDown:
    case NSEventTypeKeyUp:
    {
        const KeyboardKeys key = TranslateKey(event.keyCode);
        if (key != KeyboardKeys::UNKNOWN) inputFeed.SetKeyDown(key, event.type == NSEventTypeKeyDown);

        return true;
    }
    case NSEventTypeFlagsChanged:
    {
        const KeyboardKeys key = TranslateKey(event.keyCode);
        const NSEventModifierFlags modifierFlag = GetModifierFlag(key);

        if (key != KeyboardKeys::UNKNOWN && modifierFlag != 0) inputFeed.SetKeyDown(key, (event.modifierFlags & modifierFlag) != 0);

        return true;
    }
    case NSEventTypeLeftMouseDown:
    {
        inputFeed.SetMouseButton(MouseButtons::LEFT, true);

        return true;
    }
    case NSEventTypeLeftMouseUp:
    {
        inputFeed.SetMouseButton(MouseButtons::LEFT, false);

        return true;
    }
    case NSEventTypeRightMouseDown:
    {
        inputFeed.SetMouseButton(MouseButtons::RIGHT, true);

        return true;
    }
    case NSEventTypeRightMouseUp:
    {
        inputFeed.SetMouseButton(MouseButtons::RIGHT, false);

        return true;
    }
    case NSEventTypeOtherMouseDown:
    {
        if (event.buttonNumber == 2) inputFeed.SetMouseButton(MouseButtons::MIDDLE, true);
        else if (event.buttonNumber == 3) inputFeed.SetMouseButton(MouseButtons::X1, true);
        else if (event.buttonNumber == 4) inputFeed.SetMouseButton(MouseButtons::X2, true);

        return true;
    }
    case NSEventTypeOtherMouseUp:
    {
        if (event.buttonNumber == 2) inputFeed.SetMouseButton(MouseButtons::MIDDLE, false);
        else if (event.buttonNumber == 3) inputFeed.SetMouseButton(MouseButtons::X1, false);
        else if (event.buttonNumber == 4) inputFeed.SetMouseButton(MouseButtons::X2, false);

        return true;
    }
    case NSEventTypeMouseMoved:
    case NSEventTypeLeftMouseDragged:
    case NSEventTypeRightMouseDragged:
    case NSEventTypeOtherMouseDragged:
    {
        if (!view) return false;

        const NSPoint position = [view convertPoint:event.locationInWindow fromView:nil];
        const int32 x = static_cast<int32>(std::lround(position.x));
        const int32 y = static_cast<int32>(std::lround(view.bounds.size.height - position.y));

        inputFeed.SetMousePosition(x, y);

        return true;
    }
    case NSEventTypeScrollWheel:
    {
        const int32 delta = static_cast<int32>(std::lround(event.scrollingDeltaY * COCOA_WHEEL_DELTA));
        if (delta != 0) inputFeed.AddMouseWheel(delta);

        return true;
    }
    default: break;
    }

    return false;
}
#endif
