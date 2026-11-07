/**
 * @file mouse.cpp
 * @brief SDL2 implementation of Mouse.
 */

#include "poko/input/mouse.h"

#include <SDL.h>
#include <cstring>

namespace poko::input {

Mouse::Mouse() {
    std::memset(current_state_, 0, sizeof(current_state_));
    std::memset(previous_state_, 0, sizeof(previous_state_));
}

void Mouse::update() {
    // Store previous state
    std::memcpy(previous_state_, current_state_, sizeof(current_state_));
    previous_position_ = position_;

    // Get current button state
    uint32_t sdl_buttons = SDL_GetMouseState(nullptr, nullptr);
    current_state_[static_cast<uint32_t>(MouseButton::Left)] = (sdl_buttons & SDL_BUTTON_LMASK) ? 1 : 0;
    current_state_[static_cast<uint32_t>(MouseButton::Middle)] = (sdl_buttons & SDL_BUTTON_MMASK) ? 1 : 0;
    current_state_[static_cast<uint32_t>(MouseButton::Right)] = (sdl_buttons & SDL_BUTTON_RMASK) ? 1 : 0;
    current_state_[static_cast<uint32_t>(MouseButton::X1)] = (sdl_buttons & SDL_BUTTON_X1MASK) ? 1 : 0;
    current_state_[static_cast<uint32_t>(MouseButton::X2)] = (sdl_buttons & SDL_BUTTON_X2MASK) ? 1 : 0;

    // Get position
    int x, y;
    SDL_GetMouseState(&x, &y);
    position_.x = static_cast<float>(x);
    position_.y = static_cast<float>(y);

    // Reset scroll delta (it's set by event processing)
    scroll_delta_ = poko::math::Vec2::zero();
}

poko::math::Vec2 Mouse::position() const {
    return position_;
}

poko::math::Vec2 Mouse::relative_motion() const {
    return position_ - previous_position_;
}

poko::math::Vec2 Mouse::scroll_delta() const {
    return scroll_delta_;
}

bool Mouse::is_pressed(MouseButton button) const {
    uint32_t index = static_cast<uint32_t>(button);
    if (index >= 6) return false;
    return current_state_[index] != 0;
}

bool Mouse::was_pressed_this_frame(MouseButton button) const {
    uint32_t index = static_cast<uint32_t>(button);
    if (index >= 6) return false;
    return current_state_[index] != 0 && previous_state_[index] == 0;
}

bool Mouse::was_released_this_frame(MouseButton button) const {
    uint32_t index = static_cast<uint32_t>(button);
    if (index >= 6) return false;
    return current_state_[index] == 0 && previous_state_[index] != 0;
}

void Mouse::reset_frame_state() {
    std::memcpy(previous_state_, current_state_, sizeof(current_state_));
    previous_position_ = position_;
    scroll_delta_ = poko::math::Vec2::zero();
}

void Mouse::set_scroll_delta(float x, float y) {
    scroll_delta_.x = x;
    scroll_delta_.y = y;
}

void Mouse::inject_state(float x, float y, const uint8_t* buttons,
                         float scroll_x, float scroll_y) {
    std::memcpy(previous_state_, current_state_, sizeof(current_state_));
    previous_position_ = position_;
    if (buttons) {
        std::memcpy(current_state_, buttons, sizeof(current_state_));
    }
    position_.x = x;
    position_.y = y;
    scroll_delta_.x = scroll_x;
    scroll_delta_.y = scroll_y;
}

} // namespace poko::input
