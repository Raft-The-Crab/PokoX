/**
 * @file input_manager.cpp
 * @brief SDL2 implementation of InputManager.
 */

#include "poko/input/input_manager.h"

#include <SDL.h>

namespace poko::input {

InputManager::InputManager()
    : gamepads_{Gamepad(0), Gamepad(1), Gamepad(2), Gamepad(3)} {
}

bool InputManager::initialize() {
    // SDL should already be initialized by the platform module
    // We just need to enable gamepad events
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
        return false;
    }

    initialized_ = true;
    return true;
}

void InputManager::update() {
    if (!initialized_) {
        return;
    }

    // Update keyboard and mouse state
    keyboard_.update();
    mouse_.update();

    // Update all gamepads
    for (uint32_t i = 0; i < MAX_GAMEPADS; ++i) {
        gamepads_[i].update();
    }

    // Process SDL events for frame-specific state
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEWHEEL:
                // Update mouse scroll delta
                mouse_.set_scroll_delta(
                    static_cast<float>(event.wheel.x),
                    static_cast<float>(event.wheel.y)
                );
                break;

            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
            case SDL_CONTROLLERDEVICEREMAPPED:
                // Gamepad connection changes are handled in gamepad.update()
                break;

            default:
                // Other events are handled by the window system
                break;
        }
    }
}

void InputManager::reset_frame_state() {
    keyboard_.reset_frame_state();
    mouse_.reset_frame_state();
    for (uint32_t i = 0; i < MAX_GAMEPADS; ++i) {
        gamepads_[i].reset_frame_state();
    }
}

Keyboard& InputManager::keyboard() {
    return keyboard_;
}

const Keyboard& InputManager::keyboard() const {
    return keyboard_;
}

Mouse& InputManager::mouse() {
    return mouse_;
}

const Mouse& InputManager::mouse() const {
    return mouse_;
}

namespace {

// Shared disconnected gamepad returned for out-of-range player indices, so
// callers never silently receive a different physical device.
Gamepad& invalid_gamepad() {
    static Gamepad invalid(UINT32_MAX);
    return invalid;
}

} // namespace

Gamepad& InputManager::gamepad(uint32_t player_index) {
    if (player_index >= MAX_GAMEPADS) {
        return invalid_gamepad();
    }
    return gamepads_[player_index];
}

const Gamepad& InputManager::gamepad(uint32_t player_index) const {
    if (player_index >= MAX_GAMEPADS) {
        return invalid_gamepad();
    }
    return gamepads_[player_index];
}

void InputManager::shutdown() {
    if (initialized_) {
        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
        initialized_ = false;
    }
}

} // namespace poko::input
