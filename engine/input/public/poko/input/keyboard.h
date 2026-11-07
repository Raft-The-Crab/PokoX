/**
 * @file keyboard.h
 * @brief Keyboard state tracking.
 *
 * Provides keyboard state tracking including pressed/released state,
 * key repeat detection, and modifier key tracking.
 *
 * Usage example:
 * @code
 * Keyboard keyboard;
 * keyboard.update();  // Poll SDL for current state
 *
 * if (keyboard.is_pressed(KeyCode::W)) {
 *     // Move forward
 * }
 *
 * if (keyboard.was_pressed_this_frame(KeyCode::Space)) {
 *     // Jump
 * }
 *
 * if (has_modifier(keyboard.modifiers(), KeyModifier::Control)) {
 *     // Ctrl held
 * }
 * @endcode
 */

#ifndef POKO_INPUT_KEYBOARD_H
#define POKO_INPUT_KEYBOARD_H

#include "key_codes.h"
#include <cstdint>

namespace poko::input {

/**
 * @brief Keyboard state tracking.
 *
 * Tracks the current state of all keyboard keys, including whether
 * each key is currently pressed, and whether it was pressed or released
 * in the current frame.
 */
class Keyboard {
public:
    Keyboard();
    ~Keyboard() = default;

    /**
     * @brief Updates keyboard state from SDL.
     *
     * Should be called once per frame before querying input state.
     * This polls SDL for the current keyboard state and updates
     * the pressed/released tracking.
     */
    void update();

    /**
     * @brief Returns true if a key is currently pressed.
     *
     * @param key The key to check.
     * @return true if the key is pressed.
     */
    bool is_pressed(KeyCode key) const;

    /**
     * @brief Returns true if a key was pressed this frame.
     *
     * Useful for trigger-style actions (jump, shoot, etc.)
     *
     * @param key The key to check.
     * @return true if the key was pressed this frame.
     */
    bool was_pressed_this_frame(KeyCode key) const;

    /**
     * @brief Returns true if a key was released this frame.
     *
     * @param key The key to check.
     * @return true if the key was released this frame.
     */
    bool was_released_this_frame(KeyCode key) const;

    /**
     * @brief Returns the current modifier key flags.
     *
     * @return Combination of KeyModifier flags.
     */
    KeyModifier modifiers() const;

    /**
     * @brief Resets frame-specific state (pressed/released this frame).
     *
     * Call this at the end of the frame to clear the transient state.
     * The current pressed state is preserved.
     */
    void reset_frame_state();

    /**
     * @brief Injects a complete keyboard state, bypassing SDL polling.
     *
     * Used by VirtualInput and the input recorder playback path. The current
     * state becomes the previous state so edge detection still works.
     *
     * @param keys Array of 256 key states (nonzero = pressed).
     * @param modifiers Current modifier flags.
     */
    void inject_state(const uint8_t* keys, KeyModifier modifiers);

private:
    // Current state (256 keys to cover all possible key codes)
    uint8_t current_state_[256] = {0};
    // Previous state (for detecting press/release events)
    uint8_t previous_state_[256] = {0};
    // Current modifier flags
    KeyModifier modifiers_ = KeyModifier::None;
};

} // namespace poko::input

#endif // POKO_INPUT_KEYBOARD_H
