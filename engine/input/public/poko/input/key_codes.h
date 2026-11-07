/**
 * @file key_codes.h
 * @brief Key code definitions for keyboard input.
 *
 * This header provides a comprehensive set of key codes covering the standard
 * keyboard layout. These are mapped from SDL2 scancodes for cross-platform support.
 */

#ifndef POKO_INPUT_KEY_CODES_H
#define POKO_INPUT_KEY_CODES_H

#include <cstdint>

namespace poko::input {

/**
 * @brief Keyboard key codes.
 *
 * Covers the full standard keyboard layout including letters, numbers,
 * modifiers, function keys, and special keys.
 */
enum class KeyCode : uint32_t {
    // Unknown / reserved
    Unknown = 0,

    // Letters
    A = 1, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    // Numbers (top row)
    Num0 = 27, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,

    // Numpad
    NumPad0 = 37, NumPad1, NumPad2, NumPad3, NumPad4,
    NumPad5, NumPad6, NumPad7, NumPad8, NumPad9,
    NumPadMultiply = 47, NumPadAdd, NumPadSeparator,
    NumPadSubtract, NumPadDecimal, NumPadDivide,

    // Function keys
    F1 = 53, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

    // Modifiers
    LeftShift = 65, RightShift, LeftControl, RightControl,
    LeftAlt = 69, RightAlt, LeftGUI, RightGUI,

    // Special keys
    Escape = 73, Enter, Backspace, Tab, Space,
    CapsLock = 78, ScrollLock, NumLock, PrintScreen,
    Pause = 82, Insert, Home, PageUp, Delete, End, PageDown,

    // Arrow keys
    Left = 88, Up, Right, Down,

    // Punctuation
    Semicolon = 92, Equals, Comma, Minus, Period, Slash,
    Backquote = 98, LeftBracket, Backslash, RightBracket, Quote,

    // System keys
    Application = 103, Power, Sleep, Wake,

    // Additional navigation
    BrightnessDown = 106, BrightnessUp, DisplaySwitch,
    KBDIllumToggle, KBDIllumDown, KBDIllumUp, Eject, SleepMode,

    // Audio keys
    AudioMute = 114, AudioVolDown, AudioVolUp, MediaPlayPause,
    MediaStop, MediaNextTrack, MediaPrevTrack, MediaEject,
    Mail, MediaSelect, WWW, Calculator, Search,

    Count = 127  // Total number of key codes
};

/**
 * @brief Key modifier flags.
 */
enum class KeyModifier : uint32_t {
    None = 0,
    Shift = 1 << 0,
    Control = 1 << 1,
    Alt = 1 << 2,
    GUI = 1 << 3,
    CapsLock = 1 << 4,
    NumLock = 1 << 5,
    ScrollLock = 1 << 6
};

// Bitwise operators for KeyModifier
inline KeyModifier operator|(KeyModifier a, KeyModifier b) {
    return static_cast<KeyModifier>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline KeyModifier operator&(KeyModifier a, KeyModifier b) {
    return static_cast<KeyModifier>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline KeyModifier operator^(KeyModifier a, KeyModifier b) {
    return static_cast<KeyModifier>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
}

inline KeyModifier operator~(KeyModifier a) {
    return static_cast<KeyModifier>(~static_cast<uint32_t>(a));
}

inline bool has_modifier(KeyModifier flags, KeyModifier modifier) {
    return (flags & modifier) == modifier;
}

} // namespace poko::input

#endif // POKO_INPUT_KEY_CODES_H
