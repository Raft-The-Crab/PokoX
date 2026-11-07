/**
 * @file input_manager.h
 * @brief Central input manager for all input devices.
 *
 * Provides a unified interface for managing keyboard, mouse, and gamepad
 * input. Handles SDL event polling and state updates.
 *
 * Usage example:
 * @code
 * InputManager input;
 * input.initialize();
 *
 * // In game loop
 * while (running) {
 *     input.update();
 *
 *     if (input.keyboard().is_pressed(KeyCode::W)) {
 *         // Move forward
 *     }
 *
 *     if (input.mouse().was_pressed_this_frame(MouseButton::Left)) {
 *         // Click action
 *     }
 *
 *     Gamepad& gamepad = input.gamepad(0);
 *     if (gamepad.is_connected()) {
 *         Vec2 stick = gamepad.stick_axis(GamepadAxis::LeftStick);
 *     }
 *
 *     input.reset_frame_state();
 * }
 * @endcode
 */

#ifndef POKO_INPUT_INPUT_MANAGER_H
#define POKO_INPUT_INPUT_MANAGER_H

#include "keyboard.h"
#include "mouse.h"
#include "gamepad.h"

namespace poko::input {

/**
 * @brief Central input manager.
 *
 * Manages all input devices and coordinates state updates.
 * Provides a single point of access for keyboard, mouse, and gamepad input.
 */
class InputManager {
public:
    static constexpr uint32_t MAX_GAMEPADS = 4;  // Support up to 4 gamepads

    InputManager();
    ~InputManager() = default;

    /**
     * @brief Initializes the input manager.
     *
     * Sets up SDL for input handling. Call this once at startup.
     *
     * @return true on success.
     */
    bool initialize();

    /**
     * @brief Updates all input device states.
     *
     * Polls SDL for input events and updates keyboard, mouse, and gamepad states.
     * Call this once per frame before querying input state.
     */
    void update();

    /**
     * @brief Resets frame-specific state for all devices.
     *
     * Clears pressed/released-this-frame state and scroll deltas.
     * Call this at the end of the frame.
     */
    void reset_frame_state();

    /**
     * @brief Returns the keyboard input device.
     *
     * @return Reference to the keyboard state.
     */
    Keyboard& keyboard();

    /**
     * @brief Returns the keyboard input device (const).
     *
     * @return Const reference to the keyboard state.
     */
    const Keyboard& keyboard() const;

    /**
     * @brief Returns the mouse input device.
     *
     * @return Reference to the mouse state.
     */
    Mouse& mouse();

    /**
     * @brief Returns the mouse input device (const).
     *
     * @return Const reference to the mouse state.
     */
    const Mouse& mouse() const;

    /**
     * @brief Returns a gamepad by player index.
     *
     * @param player_index Gamepad player index (0 to MAX_GAMEPADS-1).
     * @return Reference to the gamepad state.
     */
    Gamepad& gamepad(uint32_t player_index);

    /**
     * @brief Returns a gamepad by player index (const).
     *
     * @param player_index Gamepad player index (0 to MAX_GAMEPADS-1).
     * @return Const reference to the gamepad state.
     */
    const Gamepad& gamepad(uint32_t player_index) const;

    /**
     * @brief Shuts down the input manager.
     */
    void shutdown();

private:
    Keyboard keyboard_;
    Mouse mouse_;
    Gamepad gamepads_[MAX_GAMEPADS];
    bool initialized_ = false;
};

} // namespace poko::input

#endif // POKO_INPUT_INPUT_MANAGER_H
