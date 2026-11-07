/**
 * @file input_recorder.h
 * @brief Input recording and playback system.
 *
 * Records input state for replay systems, debugging, and testing.
 * Supports recording keyboard, mouse, and gamepad input to memory or file.
 *
 * Usage example:
 * @code
 * InputRecorder recorder;
 * recorder.start_recording();
 *
 * // During gameplay
 * recorder.record_frame(keyboard, mouse, gamepads, 4);
 *
 * recorder.stop_recording();
 * recorder.save_to_file("replay.bin");
 *
 * // Playback
 * recorder.load_from_file("replay.bin");
 * recorder.start_playback();
 *
 * while (playing) {
 *     InputState state = recorder.get_next_frame();
 *     // Apply state to game
 * }
 * @endcode
 */

#ifndef POKO_INPUT_INPUT_RECORDER_H
#define POKO_INPUT_INPUT_RECORDER_H

#include "keyboard.h"
#include "mouse.h"
#include "gamepad.h"

#include <vector>
#include <cstdint>

namespace poko::input {

/**
 * @brief Single frame of recorded input state.
 */
struct InputFrame {
    uint64_t frame_number = 0;
    float delta_time = 0.0f;

    // Keyboard state (256 keys)
    uint8_t keyboard_state[256] = {0};
    KeyModifier keyboard_modifiers = KeyModifier::None;

    // Mouse state
    float mouse_x = 0.0f;
    float mouse_y = 0.0f;
    uint8_t mouse_buttons[6] = {0};  // 5 buttons
    float mouse_scroll_x = 0.0f;
    float mouse_scroll_y = 0.0f;

    // Gamepad state (up to 4 gamepads)
    struct GamepadState {
        bool connected = false;
        uint32_t buttons = 0;  // 32 bits
        float axes[7] = {0.0f};  // 7 axes
    };
    GamepadState gamepads[4];
};

/**
 * @brief Input recording and playback system.
 */
class InputRecorder {
public:
    InputRecorder();
    ~InputRecorder() = default;

    /**
     * @brief Starts recording input.
     *
     * Clears any previous recording data.
     */
    void start_recording();

    /**
     * @brief Stops recording.
     */
    void stop_recording();

    /**
     * @brief Records a single frame of input.
     *
     * @param keyboard Keyboard state.
     * @param mouse Mouse state.
     * @param gamepads Array of gamepad states.
     * @param gamepad_count Number of gamepads.
     * @param delta_time Time since last frame.
     */
    void record_frame(const Keyboard& keyboard,
                      const Mouse& mouse,
                      const Gamepad* gamepads,
                      uint32_t gamepad_count,
                      float delta_time);

    /**
     * @brief Returns true if currently recording.
     */
    bool is_recording() const;

    /**
     * @brief Returns the number of recorded frames.
     */
    uint64_t frame_count() const;

    /**
     * @brief Starts playback of recorded input.
     *
     * @param loop If true, loop the recording.
     */
    void start_playback(bool loop = false);

    /**
     * @brief Stops playback.
     */
    void stop_playback();

    /**
     * @brief Returns true if currently playing back.
     */
    bool is_playing() const;

    /**
     * @brief Gets the next frame for playback.
     *
     * @param out_frame Receives the frame data.
     * @return true if a frame was available, false if playback ended.
     */
    bool get_next_frame(InputFrame* out_frame);

    /**
     * @brief Clears all recorded data.
     */
    void clear();

    /**
     * @brief Saves recording to a file (binary format).
     *
     * @param filepath Path to save file.
     * @return true on success.
     */
    bool save_to_file(const char* filepath) const;

    /**
     * @brief Loads recording from a file.
     *
     * @param filepath Path to load file.
     * @return true on success.
     */
    bool load_from_file(const char* filepath);

private:
    std::vector<InputFrame> frames_;
    uint64_t current_frame_index_ = 0;
    bool recording_ = false;
    bool playing_ = false;
    bool loop_playback_ = false;
};

} // namespace poko::input

#endif // POKO_INPUT_INPUT_RECORDER_H
