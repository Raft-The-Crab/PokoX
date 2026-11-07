/**
 * @file virtual_input.h
 * @brief Virtual input system for automated testing.
 *
 * Allows programmatic control of input state for testing, replay systems,
 * and automated gameplay scenarios without physical input devices.
 *
 * Usage example:
 * @code
 * VirtualInput virtual;
 * virtual.set_key_pressed(KeyCode::W, true);
 * virtual.set_mouse_position(100.0f, 200.0f);
 * virtual.set_gamepad_button(0, GamepadButton::A, true);
 *
 * // Apply virtual state to real input devices
 * virtual.apply_to(keyboard, mouse, gamepads, 4);
 * @endcode
 */

#ifndef POKO_INPUT_VIRTUAL_INPUT_H
#define POKO_INPUT_VIRTUAL_INPUT_H

#include "key_codes.h"
#include "keyboard.h"
#include "mouse.h"
#include "gamepad.h"

#include <cstdint>

namespace poko::input {

/**
 * @brief Virtual input state for programmatic control.
 *
 * Provides setters for all input types, allowing automated testing
 * without physical input devices.
 */
class VirtualInput {
public:
    VirtualInput();
    ~VirtualInput() = default;

    /**
     * @brief Sets a key as pressed or released.
     *
     * @param key Key to modify.
     * @param pressed True to press, false to release.
     */
    void set_key_pressed(KeyCode key, bool pressed);

    /**
     * @brief Sets all key modifiers.
     *
     * @param modifiers Modifier flags.
     */
    void set_key_modifiers(KeyModifier modifiers);

    /**
     * @brief Sets mouse position.
     *
     * @param x X coordinate.
     * @param y Y coordinate.
     */
    void set_mouse_position(float x, float y);

    /**
     * @brief Sets a mouse button as pressed or released.
     *
     * @param button Button to modify.
     * @param pressed True to press, false to release.
     */
    void set_mouse_button(MouseButton button, bool pressed);

    /**
     * @brief Sets mouse scroll delta.
     *
     * @param x Horizontal scroll.
     * @param y Vertical scroll.
     */
    void set_mouse_scroll(float x, float y);

    /**
     * @brief Sets gamepad connection state.
     *
     * @param player_index Gamepad index.
     * @param connected True if connected.
     */
    void set_gamepad_connected(uint32_t player_index, bool connected);

    /**
     * @brief Sets a gamepad button as pressed or released.
     *
     * @param player_index Gamepad index.
     * @param button Button to modify.
     * @param pressed True to press, false to release.
     */
    void set_gamepad_button(uint32_t player_index, GamepadButton button, bool pressed);

    /**
     * @brief Sets a gamepad axis value.
     *
     * @param player_index Gamepad index.
     * @param axis Axis to modify.
     * @param value Axis value (-1.0 to 1.0 for sticks, 0.0 to 1.0 for triggers).
     */
    void set_gamepad_axis(uint32_t player_index, GamepadAxis axis, float value);

    /**
     * @brief Applies virtual state to actual input devices.
     *
     * This overrides the current device state with virtual state.
     * Useful for replay systems and automated testing.
     *
     * @param keyboard Keyboard to apply to.
     * @param mouse Mouse to apply to.
     * @param gamepads Gamepad array to apply to.
     * @param gamepad_count Number of gamepads.
     */
    void apply_to(Keyboard& keyboard, Mouse& mouse, Gamepad* gamepads, uint32_t gamepad_count);

    /**
     * @brief Clears all virtual input state.
     */
    void clear();

private:
    // Virtual keyboard state (256 keys)
    uint8_t keyboard_state_[256] = {0};
    KeyModifier keyboard_modifiers_ = KeyModifier::None;

    // Virtual mouse state
    float mouse_x_ = 0.0f;
    float mouse_y_ = 0.0f;
    uint8_t mouse_buttons_[6] = {0};
    float mouse_scroll_x_ = 0.0f;
    float mouse_scroll_y_ = 0.0f;

    // Virtual gamepad state (up to 4 gamepads)
    struct VirtualGamepad {
        bool connected = false;
        uint32_t buttons = 0;
        float axes[7] = {0.0f};
    };
    VirtualGamepad gamepads_[4];
};

} // namespace poko::input

#endif // POKO_INPUT_VIRTUAL_INPUT_H
