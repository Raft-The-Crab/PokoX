/**
 * @file gamepad.cpp
 * @brief SDL2 implementation of Gamepad.
 */

#include "poko/input/gamepad.h"

#include <SDL.h>
#include <cmath>
#include <cstring>

namespace poko::input {

Gamepad::Gamepad(uint32_t player_index)
    : player_index_(player_index) {
    for (float& axis : axes_) {
        axis = 0.0f;
    }
}

Gamepad::~Gamepad() {
    if (controller_) {
        SDL_GameControllerClose(controller_);
        controller_ = nullptr;
    }
}

// Ensures controller_ references a live SDL gamepad. The cached handle is
// dropped when the underlying device is removed, and a new controller is
// opened lazily when one appears at this player index.
SDL_GameController* Gamepad::acquire_controller() {
    if (controller_) {
        SDL_GameController* current = SDL_GameControllerFromInstanceID(instance_id_);
        if (current) {
            return current;
        }
        // Device was removed; SDL already invalidated our handle
        controller_ = nullptr;
        instance_id_ = -1;
    }

    if (SDL_IsGameController(static_cast<int>(player_index_))) {
        controller_ = SDL_GameControllerOpen(static_cast<int>(player_index_));
        if (controller_) {
            instance_id_ = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller_));
        }
    }
    return controller_;
}

void Gamepad::set_config(const Config& config) {
    config_ = config;
}

void Gamepad::update() {
    // Store previous button state
    previous_buttons_ = current_buttons_;

    SDL_GameController* controller = acquire_controller();
    if (!controller) {
        connected_ = false;
        current_buttons_ = 0;
        for (float& axis : axes_) {
            axis = 0.0f;
        }
        return;
    }

    connected_ = true;

    // Map SDL gamepad buttons to our enum
    current_buttons_ = 0;

    // Face buttons
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A)) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::A));
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B)) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::B));
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X)) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::X));
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y)) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::Y));
    }

    // D-pad
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP)) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::DPadUp));
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::DPadDown));
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::DPadLeft));
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::DPadRight));
    }

    // Shoulder buttons
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::LeftShoulder));
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::RightShoulder));
    }

    // Triggers (as buttons, for digital press detection)
    float left_trigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0f;
    float right_trigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0f;
    if (left_trigger > config_.trigger_deadzone) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::LeftTrigger));
    }
    if (right_trigger > config_.trigger_deadzone) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::RightTrigger));
    }

    // Stick buttons
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSTICK)) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::LeftStick));
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK)) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::RightStick));
    }

    // System buttons
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START)) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::Start));
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK)) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::Back));
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_GUIDE)) {
        current_buttons_ |= (1u << static_cast<uint32_t>(GamepadButton::Guide));
    }

    // Get axis values and apply deadzone
    axes_[static_cast<uint32_t>(GamepadAxis::LeftStickX)] = apply_deadzone(
        SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f,
        config_.stick_deadzone
    );
    axes_[static_cast<uint32_t>(GamepadAxis::LeftStickY)] = apply_deadzone(
        SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f,
        config_.stick_deadzone
    );
    axes_[static_cast<uint32_t>(GamepadAxis::RightStickX)] = apply_deadzone(
        SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) / 32767.0f,
        config_.stick_deadzone
    );
    axes_[static_cast<uint32_t>(GamepadAxis::RightStickY)] = apply_deadzone(
        SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) / 32767.0f,
        config_.stick_deadzone
    );
    axes_[static_cast<uint32_t>(GamepadAxis::LeftTrigger)] = apply_deadzone(
        left_trigger,
        config_.trigger_deadzone
    );
    axes_[static_cast<uint32_t>(GamepadAxis::RightTrigger)] = apply_deadzone(
        right_trigger,
        config_.trigger_deadzone
    );
}

bool Gamepad::is_connected() const {
    return connected_;
}

bool Gamepad::is_pressed(GamepadButton button) const {
    uint32_t bit = static_cast<uint32_t>(button);
    if (bit >= 32) return false;
    return (current_buttons_ & (1u << bit)) != 0;
}

bool Gamepad::was_pressed_this_frame(GamepadButton button) const {
    uint32_t bit = static_cast<uint32_t>(button);
    if (bit >= 32) return false;
    bool current = (current_buttons_ & (1u << bit)) != 0;
    bool previous = (previous_buttons_ & (1u << bit)) != 0;
    return current && !previous;
}

bool Gamepad::was_released_this_frame(GamepadButton button) const {
    uint32_t bit = static_cast<uint32_t>(button);
    if (bit >= 32) return false;
    bool current = (current_buttons_ & (1u << bit)) != 0;
    bool previous = (previous_buttons_ & (1u << bit)) != 0;
    return !current && previous;
}

float Gamepad::axis(GamepadAxis axis) const {
    uint32_t index = static_cast<uint32_t>(axis);
    if (index >= 7) return 0.0f;
    return axes_[index];
}

poko::math::Vec2 Gamepad::stick_axis(GamepadAxis stick_axis) const {
    if (stick_axis == GamepadAxis::LeftStick) {
        return poko::math::Vec2(
            axes_[static_cast<uint32_t>(GamepadAxis::LeftStickX)],
            axes_[static_cast<uint32_t>(GamepadAxis::LeftStickY)]
        );
    } else if (stick_axis == GamepadAxis::RightStick) {
        return poko::math::Vec2(
            axes_[static_cast<uint32_t>(GamepadAxis::RightStickX)],
            axes_[static_cast<uint32_t>(GamepadAxis::RightStickY)]
        );
    }
    return poko::math::Vec2::zero();
}

void Gamepad::set_rumble(float left_motor, float right_motor, uint32_t duration_ms) {
    if (!config_.enable_rumble || !connected_) {
        return;
    }

    SDL_GameController* controller = acquire_controller();
    if (!controller) {
        return;
    }

    // Clamp values to [0, 1]
    left_motor = (left_motor < 0.0f) ? 0.0f : (left_motor > 1.0f) ? 1.0f : left_motor;
    right_motor = (right_motor < 0.0f) ? 0.0f : (right_motor > 1.0f) ? 1.0f : right_motor;

    // Convert to SDL rumble range [0, 65535]
    uint16_t left_rumble = static_cast<uint16_t>(left_motor * 65535.0f);
    uint16_t right_rumble = static_cast<uint16_t>(right_motor * 65535.0f);

    SDL_GameControllerRumble(controller, left_rumble, right_rumble, duration_ms);
}

void Gamepad::reset_frame_state() {
    previous_buttons_ = current_buttons_;
}

void Gamepad::inject_state(bool connected, uint32_t buttons, const float* axes) {
    previous_buttons_ = current_buttons_;
    connected_ = connected;
    current_buttons_ = buttons;
    if (axes) {
        std::memcpy(axes_, axes, sizeof(axes_));
    }
}

float Gamepad::apply_deadzone(float value, float deadzone) const {
    if (std::abs(value) < deadzone) {
        return 0.0f;
    }
    // Remap [deadzone, 1.0] to [0.0, 1.0]
    float sign = (value < 0.0f) ? -1.0f : 1.0f;
    value = std::abs(value);
    value = (value - deadzone) / (1.0f - deadzone);
    return sign * value;
}

} // namespace poko::input
