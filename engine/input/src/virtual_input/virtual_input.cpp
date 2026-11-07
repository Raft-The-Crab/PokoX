/**
 * @file virtual_input.cpp
 * @brief Implementation of VirtualInput.
 */

#include "poko/input/virtual_input.h"

#include <cstring>

namespace poko::input {

VirtualInput::VirtualInput() {
    std::memset(keyboard_state_, 0, sizeof(keyboard_state_));
    std::memset(mouse_buttons_, 0, sizeof(mouse_buttons_));
}

void VirtualInput::set_key_pressed(KeyCode key, bool pressed) {
    uint32_t index = static_cast<uint32_t>(key);
    if (index < 256) {
        keyboard_state_[index] = pressed ? 1 : 0;
    }
}

void VirtualInput::set_key_modifiers(KeyModifier modifiers) {
    keyboard_modifiers_ = modifiers;
}

void VirtualInput::set_mouse_position(float x, float y) {
    mouse_x_ = x;
    mouse_y_ = y;
}

void VirtualInput::set_mouse_button(MouseButton button, bool pressed) {
    uint32_t index = static_cast<uint32_t>(button);
    if (index < 6) {
        mouse_buttons_[index] = pressed ? 1 : 0;
    }
}

void VirtualInput::set_mouse_scroll(float x, float y) {
    mouse_scroll_x_ = x;
    mouse_scroll_y_ = y;
}

void VirtualInput::set_gamepad_connected(uint32_t player_index, bool connected) {
    if (player_index < 4) {
        gamepads_[player_index].connected = connected;
    }
}

void VirtualInput::set_gamepad_button(uint32_t player_index, GamepadButton button, bool pressed) {
    if (player_index < 4) {
        uint32_t bit = static_cast<uint32_t>(button);
        if (bit < 32) {
            if (pressed) {
                gamepads_[player_index].buttons |= (1u << bit);
            } else {
                gamepads_[player_index].buttons &= ~(1u << bit);
            }
        }
    }
}

void VirtualInput::set_gamepad_axis(uint32_t player_index, GamepadAxis axis, float value) {
    if (player_index < 4) {
        uint32_t index = static_cast<uint32_t>(axis);
        if (index < 7) {
            gamepads_[player_index].axes[index] = value;
        }
    }
}

void VirtualInput::apply_to(Keyboard& keyboard, Mouse& mouse, Gamepad* gamepads, uint32_t gamepad_count) {
    keyboard.inject_state(keyboard_state_, keyboard_modifiers_);

    mouse.inject_state(mouse_x_, mouse_y_, mouse_buttons_,
                       mouse_scroll_x_, mouse_scroll_y_);

    uint32_t pads = (gamepad_count > 4) ? 4 : gamepad_count;
    for (uint32_t i = 0; i < pads; ++i) {
        gamepads[i].inject_state(gamepads_[i].connected, gamepads_[i].buttons,
                                 gamepads_[i].axes);
    }
}

void VirtualInput::clear() {
    std::memset(keyboard_state_, 0, sizeof(keyboard_state_));
    keyboard_modifiers_ = KeyModifier::None;
    mouse_x_ = 0.0f;
    mouse_y_ = 0.0f;
    std::memset(mouse_buttons_, 0, sizeof(mouse_buttons_));
    mouse_scroll_x_ = 0.0f;
    mouse_scroll_y_ = 0.0f;

    for (int i = 0; i < 4; ++i) {
        gamepads_[i].connected = false;
        gamepads_[i].buttons = 0;
        std::memset(gamepads_[i].axes, 0, sizeof(gamepads_[i].axes));
    }
}

} // namespace poko::input
