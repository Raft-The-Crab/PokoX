/**
 * @file keyboard.cpp
 * @brief SDL2 implementation of Keyboard.
 */

#include "poko/input/keyboard.h"

#include <SDL.h>
#include <cstring>

namespace poko::input {

Keyboard::Keyboard() {
    std::memset(current_state_, 0, sizeof(current_state_));
    std::memset(previous_state_, 0, sizeof(previous_state_));
}

void Keyboard::update() {
    // Store previous state
    std::memcpy(previous_state_, current_state_, sizeof(current_state_));

    // Get current SDL keyboard state
    const uint8_t* sdl_state = SDL_GetKeyboardState(nullptr);

    // Map SDL scancodes to our KeyCode enum
    // This is a simplified mapping for common keys
    // A full implementation would map all SDL scancodes

    // Letters A-Z
    for (int i = 0; i < 26; ++i) {
        SDL_Scancode scancode = static_cast<SDL_Scancode>(SDL_SCANCODE_A + i);
        current_state_[static_cast<uint32_t>(KeyCode::A) + i] = sdl_state[scancode];
    }

    // Numbers 0-9 (top row)
    for (int i = 0; i < 10; ++i) {
        SDL_Scancode scancode = static_cast<SDL_Scancode>(SDL_SCANCODE_0 + i);
        current_state_[static_cast<uint32_t>(KeyCode::Num0) + i] = sdl_state[scancode];
    }

    // Numpad
    for (int i = 0; i < 10; ++i) {
        SDL_Scancode scancode = static_cast<SDL_Scancode>(SDL_SCANCODE_KP_0 + i);
        current_state_[static_cast<uint32_t>(KeyCode::NumPad0) + i] = sdl_state[scancode];
    }
    current_state_[static_cast<uint32_t>(KeyCode::NumPadMultiply)] = sdl_state[SDL_SCANCODE_KP_MULTIPLY];
    current_state_[static_cast<uint32_t>(KeyCode::NumPadAdd)] = sdl_state[SDL_SCANCODE_KP_PLUS];
    current_state_[static_cast<uint32_t>(KeyCode::NumPadSeparator)] = sdl_state[SDL_SCANCODE_KP_ENTER];
    current_state_[static_cast<uint32_t>(KeyCode::NumPadSubtract)] = sdl_state[SDL_SCANCODE_KP_MINUS];
    current_state_[static_cast<uint32_t>(KeyCode::NumPadDecimal)] = sdl_state[SDL_SCANCODE_KP_PERIOD];
    current_state_[static_cast<uint32_t>(KeyCode::NumPadDivide)] = sdl_state[SDL_SCANCODE_KP_DIVIDE];

    // Function keys
    for (int i = 0; i < 12; ++i) {
        SDL_Scancode scancode = static_cast<SDL_Scancode>(SDL_SCANCODE_F1 + i);
        current_state_[static_cast<uint32_t>(KeyCode::F1) + i] = sdl_state[scancode];
    }

    // Modifiers
    current_state_[static_cast<uint32_t>(KeyCode::LeftShift)] = sdl_state[SDL_SCANCODE_LSHIFT];
    current_state_[static_cast<uint32_t>(KeyCode::RightShift)] = sdl_state[SDL_SCANCODE_RSHIFT];
    current_state_[static_cast<uint32_t>(KeyCode::LeftControl)] = sdl_state[SDL_SCANCODE_LCTRL];
    current_state_[static_cast<uint32_t>(KeyCode::RightControl)] = sdl_state[SDL_SCANCODE_RCTRL];
    current_state_[static_cast<uint32_t>(KeyCode::LeftAlt)] = sdl_state[SDL_SCANCODE_LALT];
    current_state_[static_cast<uint32_t>(KeyCode::RightAlt)] = sdl_state[SDL_SCANCODE_RALT];
    current_state_[static_cast<uint32_t>(KeyCode::LeftGUI)] = sdl_state[SDL_SCANCODE_LGUI];
    current_state_[static_cast<uint32_t>(KeyCode::RightGUI)] = sdl_state[SDL_SCANCODE_RGUI];

    // Special keys
    current_state_[static_cast<uint32_t>(KeyCode::Escape)] = sdl_state[SDL_SCANCODE_ESCAPE];
    current_state_[static_cast<uint32_t>(KeyCode::Enter)] = sdl_state[SDL_SCANCODE_RETURN];
    current_state_[static_cast<uint32_t>(KeyCode::Backspace)] = sdl_state[SDL_SCANCODE_BACKSPACE];
    current_state_[static_cast<uint32_t>(KeyCode::Tab)] = sdl_state[SDL_SCANCODE_TAB];
    current_state_[static_cast<uint32_t>(KeyCode::Space)] = sdl_state[SDL_SCANCODE_SPACE];
    current_state_[static_cast<uint32_t>(KeyCode::CapsLock)] = sdl_state[SDL_SCANCODE_CAPSLOCK];
    current_state_[static_cast<uint32_t>(KeyCode::ScrollLock)] = sdl_state[SDL_SCANCODE_SCROLLLOCK];
    current_state_[static_cast<uint32_t>(KeyCode::NumLock)] = sdl_state[SDL_SCANCODE_NUMLOCKCLEAR];
    current_state_[static_cast<uint32_t>(KeyCode::PrintScreen)] = sdl_state[SDL_SCANCODE_PRINTSCREEN];
    current_state_[static_cast<uint32_t>(KeyCode::Pause)] = sdl_state[SDL_SCANCODE_PAUSE];
    current_state_[static_cast<uint32_t>(KeyCode::Insert)] = sdl_state[SDL_SCANCODE_INSERT];
    current_state_[static_cast<uint32_t>(KeyCode::Home)] = sdl_state[SDL_SCANCODE_HOME];
    current_state_[static_cast<uint32_t>(KeyCode::PageUp)] = sdl_state[SDL_SCANCODE_PAGEUP];
    current_state_[static_cast<uint32_t>(KeyCode::Delete)] = sdl_state[SDL_SCANCODE_DELETE];
    current_state_[static_cast<uint32_t>(KeyCode::End)] = sdl_state[SDL_SCANCODE_END];
    current_state_[static_cast<uint32_t>(KeyCode::PageDown)] = sdl_state[SDL_SCANCODE_PAGEDOWN];

    // Arrow keys
    current_state_[static_cast<uint32_t>(KeyCode::Left)] = sdl_state[SDL_SCANCODE_LEFT];
    current_state_[static_cast<uint32_t>(KeyCode::Up)] = sdl_state[SDL_SCANCODE_UP];
    current_state_[static_cast<uint32_t>(KeyCode::Right)] = sdl_state[SDL_SCANCODE_RIGHT];
    current_state_[static_cast<uint32_t>(KeyCode::Down)] = sdl_state[SDL_SCANCODE_DOWN];

    // Punctuation
    current_state_[static_cast<uint32_t>(KeyCode::Semicolon)] = sdl_state[SDL_SCANCODE_SEMICOLON];
    current_state_[static_cast<uint32_t>(KeyCode::Equals)] = sdl_state[SDL_SCANCODE_EQUALS];
    current_state_[static_cast<uint32_t>(KeyCode::Comma)] = sdl_state[SDL_SCANCODE_COMMA];
    current_state_[static_cast<uint32_t>(KeyCode::Minus)] = sdl_state[SDL_SCANCODE_MINUS];
    current_state_[static_cast<uint32_t>(KeyCode::Period)] = sdl_state[SDL_SCANCODE_PERIOD];
    current_state_[static_cast<uint32_t>(KeyCode::Slash)] = sdl_state[SDL_SCANCODE_SLASH];
    current_state_[static_cast<uint32_t>(KeyCode::Backquote)] = sdl_state[SDL_SCANCODE_GRAVE];
    current_state_[static_cast<uint32_t>(KeyCode::LeftBracket)] = sdl_state[SDL_SCANCODE_LEFTBRACKET];
    current_state_[static_cast<uint32_t>(KeyCode::Backslash)] = sdl_state[SDL_SCANCODE_BACKSLASH];
    current_state_[static_cast<uint32_t>(KeyCode::RightBracket)] = sdl_state[SDL_SCANCODE_RIGHTBRACKET];
    current_state_[static_cast<uint32_t>(KeyCode::Quote)] = sdl_state[SDL_SCANCODE_APOSTROPHE];

    // System and media keys
    current_state_[static_cast<uint32_t>(KeyCode::Application)] = sdl_state[SDL_SCANCODE_APPLICATION];
    current_state_[static_cast<uint32_t>(KeyCode::Power)] = sdl_state[SDL_SCANCODE_POWER];
    current_state_[static_cast<uint32_t>(KeyCode::Sleep)] = sdl_state[SDL_SCANCODE_SLEEP];
    current_state_[static_cast<uint32_t>(KeyCode::BrightnessDown)] = sdl_state[SDL_SCANCODE_BRIGHTNESSDOWN];
    current_state_[static_cast<uint32_t>(KeyCode::BrightnessUp)] = sdl_state[SDL_SCANCODE_BRIGHTNESSUP];
    current_state_[static_cast<uint32_t>(KeyCode::DisplaySwitch)] = sdl_state[SDL_SCANCODE_DISPLAYSWITCH];
    current_state_[static_cast<uint32_t>(KeyCode::KBDIllumToggle)] = sdl_state[SDL_SCANCODE_KBDILLUMTOGGLE];
    current_state_[static_cast<uint32_t>(KeyCode::KBDIllumDown)] = sdl_state[SDL_SCANCODE_KBDILLUMDOWN];
    current_state_[static_cast<uint32_t>(KeyCode::KBDIllumUp)] = sdl_state[SDL_SCANCODE_KBDILLUMUP];
    current_state_[static_cast<uint32_t>(KeyCode::Eject)] = sdl_state[SDL_SCANCODE_EJECT];
    current_state_[static_cast<uint32_t>(KeyCode::AudioMute)] = sdl_state[SDL_SCANCODE_AUDIOMUTE];
    current_state_[static_cast<uint32_t>(KeyCode::AudioVolDown)] = sdl_state[SDL_SCANCODE_VOLUMEDOWN];
    current_state_[static_cast<uint32_t>(KeyCode::AudioVolUp)] = sdl_state[SDL_SCANCODE_VOLUMEUP];
    current_state_[static_cast<uint32_t>(KeyCode::MediaStop)] = sdl_state[SDL_SCANCODE_AUDIOSTOP];
    current_state_[static_cast<uint32_t>(KeyCode::MediaNextTrack)] = sdl_state[SDL_SCANCODE_AUDIONEXT];
    current_state_[static_cast<uint32_t>(KeyCode::MediaPrevTrack)] = sdl_state[SDL_SCANCODE_AUDIOPREV];
    current_state_[static_cast<uint32_t>(KeyCode::Mail)] = sdl_state[SDL_SCANCODE_MAIL];
    current_state_[static_cast<uint32_t>(KeyCode::MediaSelect)] = sdl_state[SDL_SCANCODE_MEDIASELECT];
    current_state_[static_cast<uint32_t>(KeyCode::WWW)] = sdl_state[SDL_SCANCODE_WWW];
    current_state_[static_cast<uint32_t>(KeyCode::Calculator)] = sdl_state[SDL_SCANCODE_CALCULATOR];

    // Update modifier flags
    modifiers_ = KeyModifier::None;
    if (sdl_state[SDL_SCANCODE_LSHIFT] || sdl_state[SDL_SCANCODE_RSHIFT]) {
        modifiers_ = modifiers_ | KeyModifier::Shift;
    }
    if (sdl_state[SDL_SCANCODE_LCTRL] || sdl_state[SDL_SCANCODE_RCTRL]) {
        modifiers_ = modifiers_ | KeyModifier::Control;
    }
    if (sdl_state[SDL_SCANCODE_LALT] || sdl_state[SDL_SCANCODE_RALT]) {
        modifiers_ = modifiers_ | KeyModifier::Alt;
    }
    if (sdl_state[SDL_SCANCODE_LGUI] || sdl_state[SDL_SCANCODE_RGUI]) {
        modifiers_ = modifiers_ | KeyModifier::GUI;
    }
    if (SDL_GetModState() & KMOD_CAPS) {
        modifiers_ = modifiers_ | KeyModifier::CapsLock;
    }
    if (SDL_GetModState() & KMOD_NUM) {
        modifiers_ = modifiers_ | KeyModifier::NumLock;
    }
    if (SDL_GetModState() & KMOD_SCROLL) {
        modifiers_ = modifiers_ | KeyModifier::ScrollLock;
    }
}

bool Keyboard::is_pressed(KeyCode key) const {
    uint32_t index = static_cast<uint32_t>(key);
    if (index >= 256) return false;
    return current_state_[index] != 0;
}

bool Keyboard::was_pressed_this_frame(KeyCode key) const {
    uint32_t index = static_cast<uint32_t>(key);
    if (index >= 256) return false;
    return current_state_[index] != 0 && previous_state_[index] == 0;
}

bool Keyboard::was_released_this_frame(KeyCode key) const {
    uint32_t index = static_cast<uint32_t>(key);
    if (index >= 256) return false;
    return current_state_[index] == 0 && previous_state_[index] != 0;
}

KeyModifier Keyboard::modifiers() const {
    return modifiers_;
}

void Keyboard::reset_frame_state() {
    // Copy current to previous for next frame comparison
    std::memcpy(previous_state_, current_state_, sizeof(current_state_));
}

void Keyboard::inject_state(const uint8_t* keys, KeyModifier modifiers) {
    std::memcpy(previous_state_, current_state_, sizeof(current_state_));
    if (keys) {
        std::memcpy(current_state_, keys, sizeof(current_state_));
    }
    modifiers_ = modifiers;
}

} // namespace poko::input
