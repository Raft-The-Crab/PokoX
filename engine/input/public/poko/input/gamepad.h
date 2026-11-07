/**
 * @file gamepad.h
 * @brief Gamepad (controller) state tracking.
 *
 * Provides gamepad state tracking including buttons, analog axes,
 * trigger values, and rumble support.
 *
 * Usage example:
 * @code
 * Gamepad gamepad(0);  // Use first gamepad
 * gamepad.update();
 *
 * if (gamepad.is_connected()) {
 *     if (gamepad.is_pressed(GamepadButton::A)) {
 *         // Action button
 *     }
 *
 *     Vec2 left_stick = gamepad.axis(GamepadAxis::LeftStick);
 *     if (left_stick.length() > 0.5f) {
 *         // Move character
 *     }
 * }
 * @endcode
 */

#ifndef POKO_INPUT_GAMEPAD_H
#define POKO_INPUT_GAMEPAD_H

#include "poko/math/vec2.h"
#include <cstdint>

struct _SDL_GameController;

namespace poko::input {

/**
 * @brief Gamepad button codes (standard Xbox/PlayStation layout).
 */
enum class GamepadButton : uint32_t {
    Unknown = 0,
    // Face buttons
    A = 1, B, X, Y,
    // D-pad
    DPadUp = 5, DPadDown, DPadLeft, DPadRight,
    // Shoulder buttons
    LeftShoulder = 9, RightShoulder,
    // Triggers (treated as buttons for digital press)
    LeftTrigger = 11, RightTrigger,
    // Stick buttons (clicking the stick)
    LeftStick = 13, RightStick,
    // System buttons
    Start = 15, Back, Guide,
    // Extra buttons
    LeftStickUp = 18, LeftStickDown, LeftStickLeft, LeftStickRight,
    RightStickUp = 22, RightStickDown, RightStickLeft, RightStickRight,
    Count = 26
};

/**
 * @brief Gamepad axis codes.
 */
enum class GamepadAxis : uint32_t {
    Unknown = 0,
    LeftStickX = 1,
    LeftStickY = 2,
    RightStickX = 3,
    RightStickY = 4,
    LeftTrigger = 5,  // Analog trigger value (0.0 to 1.0)
    RightTrigger = 6,
    LeftStick = 7,    // Convenience value for stick_axis()
    RightStick = 8,   // Convenience value for stick_axis()
    Count = 9
};

/**
 * @brief Gamepad state tracking.
 *
 * Tracks gamepad button state, analog axes, trigger values,
 * and connection status. Supports deadzone configuration.
 */
class Gamepad {
public:
    /**
     * @brief Gamepad configuration.
     */
    struct Config {
        float stick_deadzone = 0.15f;      // Deadzone for analog sticks
        float trigger_deadzone = 0.05f;    // Deadzone for triggers
        bool enable_rumble = true;         // Rumble support
    };

    /**
     * @brief Constructor.
     *
     * @param player_index Gamepad player index (0-based).
     */
    explicit Gamepad(uint32_t player_index = 0);
    ~Gamepad();

    /**
     * @brief Sets the gamepad configuration.
     *
     * @param config Configuration parameters.
     */
    void set_config(const Config& config);

    /**
     * @brief Updates gamepad state from SDL.
     *
     * Should be called once per frame before querying input state.
     * This polls SDL for the current gamepad state.
     */
    void update();

    /**
     * @brief Returns true if the gamepad is connected.
     *
     * @return true if a gamepad is connected at this player index.
     */
    bool is_connected() const;

    /**
     * @brief Returns true if a button is currently pressed.
     *
     * @param button The button to check.
     * @return true if the button is pressed.
     */
    bool is_pressed(GamepadButton button) const;

    /**
     * @brief Returns true if a button was pressed this frame.
     *
     * @param button The button to check.
     * @return true if the button was pressed this frame.
     */
    bool was_pressed_this_frame(GamepadButton button) const;

    /**
     * @brief Returns true if a button was released this frame.
     *
     * @param button The button to check.
     * @return true if the button was released this frame.
     */
    bool was_released_this_frame(GamepadButton button) const;

    /**
     * @brief Returns the value of an analog axis.
     *
     * Values are in the range [-1.0, 1.0] for sticks, and [0.0, 1.0] for triggers.
     * Deadzone is applied automatically.
     *
     * @param axis The axis to query.
     * @return Axis value.
     */
    float axis(GamepadAxis axis) const;

    /**
     * @brief Returns the 2D value of a stick axis.
     *
     * Combines X and Y axes into a Vec2 for convenience.
     *
     * @param stick_axis The stick (LeftStick or RightStick).
     * @return Stick direction (x, y) with deadzone applied.
     */
    poko::math::Vec2 stick_axis(GamepadAxis stick_axis) const;

    /**
     * @brief Sets rumble (vibration) on the gamepad.
     *
     * @param left_motor Left motor intensity (0.0 to 1.0).
     * @param right_motor Right motor intensity (0.0 to 1.0).
     * @param duration_ms Duration in milliseconds (0 for continuous).
     */
    void set_rumble(float left_motor, float right_motor, uint32_t duration_ms = 0);

    /**
     * @brief Resets frame-specific state (pressed/released this frame).
     *
     * Call this at the end of the frame to clear the transient state.
     * The current pressed state is preserved.
     */
    void reset_frame_state();

    /**
     * @brief Injects a complete gamepad state, bypassing SDL polling.
     *
     * Used by VirtualInput and input playback. The previous button state is
     * rolled forward so edge detection still works.
     */
    void inject_state(bool connected, uint32_t buttons, const float* axes);

private:
    uint32_t player_index_;
    Config config_;

    // Connection state
    bool connected_ = false;

    // Button state (32 buttons)
    uint32_t current_buttons_ = 0;
    uint32_t previous_buttons_ = 0;

    // Axis values (7 axes)
    float axes_[7] = {0.0f};

    // Cached SDL controller handle (opened lazily, closed on destruction or
    // device removal). player_index_ doubles as the joystick device index.
    _SDL_GameController* controller_ = nullptr;
    int instance_id_ = -1;

    // Apply deadzone to axis value
    float apply_deadzone(float value, float deadzone) const;

    // Returns a live controller handle, opening one lazily if needed.
    _SDL_GameController* acquire_controller();
};

} // namespace poko::input

#endif // POKO_INPUT_GAMEPAD_H
