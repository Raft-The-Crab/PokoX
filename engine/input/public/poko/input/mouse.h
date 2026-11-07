/**
 * @file mouse.h
 * @brief Mouse state tracking.
 *
 * Provides mouse state tracking including position, button state,
 * scroll wheel delta, and relative motion.
 *
 * Usage example:
 * @code
 * Mouse mouse;
 * mouse.update();  // Poll SDL for current state
 *
 * if (mouse.is_pressed(MouseButton::Left)) {
 *     // Drag or click action
 * }
 *
 * Vec2 pos = mouse.position();
 * float scroll = mouse.scroll_delta();
 *
 * if (mouse.was_pressed_this_frame(MouseButton::Right)) {
 *     // Context menu
 * }
 * @endcode
 */

#ifndef POKO_INPUT_MOUSE_H
#define POKO_INPUT_MOUSE_H

#include "poko/math/vec2.h"
#include <cstdint>

namespace poko::input {

/**
 * @brief Mouse button codes.
 */
enum class MouseButton : uint32_t {
    Unknown = 0,
    Left = 1,
    Middle = 2,
    Right = 3,
    X1 = 4,  // Extra button 1
    X2 = 5,  // Extra button 2
    Count = 6
};

/**
 * @brief Mouse state tracking.
 *
 * Tracks mouse position, button state, scroll wheel delta,
 * and relative motion (for camera controls, etc.).
 */
class Mouse {
public:
    Mouse();
    ~Mouse() = default;

    /**
     * @brief Updates mouse state from SDL.
     *
     * Should be called once per frame before querying input state.
     * This polls SDL for the current mouse state and updates
     * the pressed/released tracking.
     */
    void update();

    /**
     * @brief Returns the current mouse position in window coordinates.
     *
     * @return Mouse position (x, y) in pixels from top-left.
     */
    poko::math::Vec2 position() const;

    /**
     * @brief Returns the mouse motion since last frame.
     *
     * Useful for camera rotation or pointer-based controls.
     *
     * @return Relative motion (dx, dy) in pixels.
     */
    poko::math::Vec2 relative_motion() const;

    /**
     * @brief Returns the scroll wheel delta this frame.
     *
     * Positive values scroll up, negative values scroll down.
     *
     * @return Scroll delta (x, y) where y is vertical scroll.
     */
    poko::math::Vec2 scroll_delta() const;

    /**
     * @brief Returns true if a mouse button is currently pressed.
     *
     * @param button The button to check.
     * @return true if the button is pressed.
     */
    bool is_pressed(MouseButton button) const;

    /**
     * @brief Returns true if a button was pressed this frame.
     *
     * Useful for click detection.
     *
     * @param button The button to check.
     * @return true if the button was pressed this frame.
     */
    bool was_pressed_this_frame(MouseButton button) const;

    /**
     * @brief Returns true if a button was released this frame.
     *
     * @param button The button to check.
     * @return true if the button was released this frame.
     */
    bool was_released_this_frame(MouseButton button) const;

    /**
     * @brief Resets frame-specific state (pressed/released this frame, scroll).
     *
     * Call this at the end of the frame to clear the transient state.
     * The current pressed state is preserved.
     */
    void reset_frame_state();

    /**
     * @brief Sets the scroll delta (called by InputManager from SDL events).
     *
     * @param x Horizontal scroll delta.
     * @param y Vertical scroll delta.
     */
    void set_scroll_delta(float x, float y);

    /**
     * @brief Injects a complete mouse state, bypassing SDL polling.
     *
     * Used by VirtualInput and input playback. The previous state is rolled
     * forward so edge detection still works.
     */
    void inject_state(float x, float y, const uint8_t* buttons,
                      float scroll_x, float scroll_y);

private:
    // Current button state
    uint8_t current_state_[6] = {0};  // 6 mouse buttons
    // Previous button state
    uint8_t previous_state_[6] = {0};
    // Position
    poko::math::Vec2 position_ = poko::math::Vec2::zero();
    // Previous position (for relative motion)
    poko::math::Vec2 previous_position_ = poko::math::Vec2::zero();
    // Scroll delta this frame
    poko::math::Vec2 scroll_delta_ = poko::math::Vec2::zero();
};

} // namespace poko::input

#endif // POKO_INPUT_MOUSE_H
