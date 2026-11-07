/**
 * @file input_recorder.cpp
 * @brief Implementation of InputRecorder.
 */

#include "poko/input/input_recorder.h"

#include <cstring>
#include <cstdio>

namespace poko::input {

InputRecorder::InputRecorder()
    : current_frame_index_(0)
    , recording_(false)
    , playing_(false)
    , loop_playback_(false) {
}

void InputRecorder::start_recording() {
    frames_.clear();
    current_frame_index_ = 0;
    recording_ = true;
    playing_ = false;
}

void InputRecorder::stop_recording() {
    recording_ = false;
}

void InputRecorder::record_frame(const Keyboard& keyboard,
                                  const Mouse& mouse,
                                  const Gamepad* gamepads,
                                  uint32_t gamepad_count,
                                  float delta_time) {
    if (!recording_) {
        return;
    }

    InputFrame frame;
    frame.frame_number = frames_.size();
    frame.delta_time = delta_time;

    // Record the complete keyboard state (all defined key codes)
    for (uint32_t i = 1; i < static_cast<uint32_t>(KeyCode::Count); ++i) {
        frame.keyboard_state[i] = keyboard.is_pressed(static_cast<KeyCode>(i)) ? 1 : 0;
    }
    frame.keyboard_modifiers = keyboard.modifiers();

    // Record mouse state
    auto mouse_pos = mouse.position();
    frame.mouse_x = mouse_pos.x;
    frame.mouse_y = mouse_pos.y;
    for (uint32_t i = 0; i < 6; ++i) {
        frame.mouse_buttons[i] =
            mouse.is_pressed(static_cast<MouseButton>(i)) ? 1 : 0;
    }
    auto scroll = mouse.scroll_delta();
    frame.mouse_scroll_x = scroll.x;
    frame.mouse_scroll_y = scroll.y;

    // Record gamepad state
    uint32_t pads_to_record = (gamepad_count > 4) ? 4 : gamepad_count;
    for (uint32_t i = 0; i < pads_to_record; ++i) {
        frame.gamepads[i].connected = gamepads[i].is_connected();
        if (gamepads[i].is_connected()) {
            for (uint32_t b = 1; b < static_cast<uint32_t>(GamepadButton::Count); ++b) {
                if (gamepads[i].is_pressed(static_cast<GamepadButton>(b))) {
                    frame.gamepads[i].buttons |= (1u << b);
                }
            }
            for (uint32_t a = 1; a <= 6; ++a) {
                frame.gamepads[i].axes[a] =
                    gamepads[i].axis(static_cast<GamepadAxis>(a));
            }
        }
    }

    frames_.push_back(frame);
}

bool InputRecorder::is_recording() const {
    return recording_;
}

uint64_t InputRecorder::frame_count() const {
    return frames_.size();
}

void InputRecorder::start_playback(bool loop) {
    current_frame_index_ = 0;
    playing_ = true;
    recording_ = false;
    loop_playback_ = loop;
}

void InputRecorder::stop_playback() {
    playing_ = false;
}

bool InputRecorder::is_playing() const {
    return playing_;
}

bool InputRecorder::get_next_frame(InputFrame* out_frame) {
    if (!playing_ || frames_.empty()) {
        return false;
    }

    if (current_frame_index_ >= frames_.size()) {
        if (loop_playback_) {
            current_frame_index_ = 0;
        } else {
            playing_ = false;
            return false;
        }
    }

    *out_frame = frames_[current_frame_index_];
    current_frame_index_++;
    return true;
}

void InputRecorder::clear() {
    frames_.clear();
    current_frame_index_ = 0;
    recording_ = false;
    playing_ = false;
}

bool InputRecorder::save_to_file(const char* filepath) const {
    FILE* file = std::fopen(filepath, "wb");
    if (!file) {
        return false;
    }

    // Write header
    uint64_t frame_count = frames_.size();
    std::fwrite(&frame_count, sizeof(uint64_t), 1, file);

    // Write frames
    for (const auto& frame : frames_) {
        std::fwrite(&frame, sizeof(InputFrame), 1, file);
    }

    std::fclose(file);
    return true;
}

bool InputRecorder::load_from_file(const char* filepath) {
    FILE* file = std::fopen(filepath, "rb");
    if (!file) {
        return false;
    }

    // Read header
    uint64_t frame_count = 0;
    if (std::fread(&frame_count, sizeof(uint64_t), 1, file) != 1) {
        std::fclose(file);
        return false;
    }

    // Read frames
    frames_.resize(frame_count);
    for (uint64_t i = 0; i < frame_count; ++i) {
        if (std::fread(&frames_[i], sizeof(InputFrame), 1, file) != 1) {
            std::fclose(file);
            return false;
        }
    }

    std::fclose(file);
    current_frame_index_ = 0;
    return true;
}

} // namespace poko::input
