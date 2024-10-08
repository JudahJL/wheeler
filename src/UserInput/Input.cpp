#include "UserInput/Input.h"

#include <WinUser.h>
#include <Windows.h>
#include <dinput.h>

// #include <imgui.h>

#include "Wheeler/Wheeler.h"
#include "UserInput/Controls.h"
#define IM_VK_KEYPAD_ENTER (VK_RETURN + 256)

class CharEvent final: public RE::InputEvent
{
public:
    uint32_t keyCode;  // 18 (ascii code)
};

enum : std::uint32_t {
    kInvalid        = static_cast<std::uint32_t>(-1),
    kKeyboardOffset = 0,
    kMouseOffset    = 256,
    kGamepadOffset  = 266
};

static std::uint32_t GetGamepadIndex(RE::BSWin32GamepadDevice::Key a_key) {
    using Key = RE::BSWin32GamepadDevice::Key;

    std::uint32_t index;
    switch(a_key) {
        case Key::kUp:
            index = 0;
            break;
        case Key::kDown:
            index = 1;
            break;
        case Key::kLeft:
            index = 2;
            break;
        case Key::kRight:
            index = 3;
            break;
        case Key::kStart:
            index = 4;
            break;
        case Key::kBack:
            index = 5;
            break;
        case Key::kLeftThumb:
            index = 6;
            break;
        case Key::kRightThumb:
            index = 7;
            break;
        case Key::kLeftShoulder:
            index = 8;
            break;
        case Key::kRightShoulder:
            index = 9;
            break;
        case Key::kA:
            index = 10;
            break;
        case Key::kB:
            index = 11;
            break;
        case Key::kX:
            index = 12;
            break;
        case Key::kY:
            index = 13;
            break;
        case Key::kLeftTrigger:
            index = 14;
            break;
        case Key::kRightTrigger:
            index = 15;
            break;
        default:
            index = kInvalid;
            break;
    }

    return index != kInvalid ? index + kGamepadOffset : kInvalid;
}

void Input::ProcessAndFilter(RE::InputEvent** a_event) {
    if(!a_event) {
        return;
    }
    const bool      wheelerOpen{ Wheeler::IsWheelerOpen() };
    RE::InputEvent* event{ *a_event };
    RE::InputEvent* prev{ nullptr };
    while(event != nullptr) {
        bool shouldDispatch = true;
        // update cursor pos only if the wheel is open, and hence block dispatch of thumbstick/mouse event to game
        if(event->eventType.all(RE::INPUT_EVENT_TYPE::kMouseMove)) {
            if(wheelerOpen) {
                const auto* mouseMove{ event->AsMouseMoveEvent() };
                Wheeler::UpdateCursorPosMouse(mouseMove->mouseInputX, mouseMove->mouseInputY);
                shouldDispatch = false;  // block mouse input when wheel is open
            }
        } else if(event->eventType.all(RE::INPUT_EVENT_TYPE::kThumbstick)) {
            if(wheelerOpen) {
                const auto* thumbstick{ event->AsThumbstickEvent() };
                if(thumbstick->IsRight()) {
                    Wheeler::UpdateCursorPosGamepad(thumbstick->xValue, thumbstick->yValue);
                    shouldDispatch = false;  // block thumbstick right input when wheel is open
                }
            }
        } else if(event->eventType.all(RE::INPUT_EVENT_TYPE::kButton)) {
            const auto* button{ event->AsButtonEvent() };
            if(button) {
                uint32_t input{ button->GetIDCode() };
                using DeviceType = RE::INPUT_DEVICE;
                bool                   isGamePad{ false };
                const RE::INPUT_DEVICE device{ button->device.get() };
                switch(device) {
                    case DeviceType::kMouse:
                        input += kMouseOffset;
                        break;
                    case DeviceType::kKeyboard:
                        input += kKeyboardOffset;
                        break;
                    case DeviceType::kGamepad:
                        input     = GetGamepadIndex(static_cast<RE::BSWin32GamepadDevice::Key>(input));
                        isGamePad = true;
                        break;
                    default:
                        break;
                }
                const bool isKeyBound = Controls::IsKeyBound(input);
                if(wheelerOpen) {
                    if(isKeyBound) {
                        shouldDispatch = false;  // block button input when wheel is open, regardless of whether it's down, up, pressed
                    } else {
                        const auto* ctrlMap{ RE::ControlMap::GetSingleton() };
                        if(ctrlMap) [[likely]] {  // filter out inputs like opening tween menu to avoid some consistency issues
                            shouldDispatch = !std::ranges::contains(EventsToFilterWhenWheelerActive, ctrlMap->GetUserEventName(input, device));
                        }
                    }
                }
                if(isKeyBound) {
                    if(button->IsDown() || button->IsUp()) {
                        // dispatch no matter if wheeler is open, wheeler will handle dispatched logic.
                        Controls::Dispatch(input, button->IsDown(), isGamePad);
                    }
                }
            }
        }

        RE::InputEvent* nextEvent{ event->next };
        if(!shouldDispatch) {
            if(prev != nullptr) {
                prev->next = nextEvent;
            } else {
                *a_event = nextEvent;
            }
        } else {
            prev = event;
        }
        event = nextEvent;
    }
}

