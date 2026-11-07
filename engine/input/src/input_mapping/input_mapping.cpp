/**
 * @file input_mapping.cpp
 * @brief Implementation of InputMapping.
 */

#include "poko/input/input_mapping.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace poko::input {

InputMapping::InputMapping() {
}

void InputMapping::map_action(const std::string& action, KeyCode key) {
    InputBinding binding;
    binding.type = BindingType::Key;
    binding.key = key;
    bindings_[action].push_back(binding);
}

void InputMapping::map_action(const std::string& action, MouseButton button) {
    InputBinding binding;
    binding.type = BindingType::MouseButton;
    binding.mouse_button = button;
    bindings_[action].push_back(binding);
}

void InputMapping::map_action(const std::string& action, uint32_t gamepad_index, GamepadButton button) {
    InputBinding binding;
    binding.type = BindingType::GamepadButton;
    binding.gamepad_index = gamepad_index;
    binding.gamepad_button = button;
    bindings_[action].push_back(binding);
}

void InputMapping::unmap_action(const std::string& action) {
    bindings_.erase(action);
}

void InputMapping::unmap_binding(const std::string& action, const InputBinding& binding) {
    auto it = bindings_.find(action);
    if (it != bindings_.end()) {
        auto& bindings = it->second;
        for (auto bi = bindings.begin(); bi != bindings.end(); ++bi) {
            if (bi->type == binding.type &&
                bi->key == binding.key &&
                bi->mouse_button == binding.mouse_button &&
                bi->gamepad_index == binding.gamepad_index &&
                bi->gamepad_button == binding.gamepad_button) {
                bindings.erase(bi);
                break;
            }
        }
        if (bindings.empty()) {
            bindings_.erase(it);
        }
    }
}

bool InputMapping::is_action_pressed(const std::string& action,
                                      const Keyboard& keyboard,
                                      const Mouse& mouse,
                                      const Gamepad* gamepads,
                                      uint32_t gamepad_count) const {
    auto it = bindings_.find(action);
    if (it == bindings_.end()) {
        return false;
    }

    for (const auto& binding : it->second) {
        switch (binding.type) {
            case BindingType::Key:
                if (keyboard.is_pressed(binding.key)) {
                    return true;
                }
                break;
            case BindingType::MouseButton:
                if (mouse.is_pressed(binding.mouse_button)) {
                    return true;
                }
                break;
            case BindingType::GamepadButton:
                if (binding.gamepad_index < gamepad_count) {
                    if (gamepads[binding.gamepad_index].is_pressed(binding.gamepad_button)) {
                        return true;
                    }
                }
                break;
            default:
                break;
        }
    }

    return false;
}

bool InputMapping::was_action_pressed_this_frame(const std::string& action,
                                                  const Keyboard& keyboard,
                                                  const Mouse& mouse,
                                                  const Gamepad* gamepads,
                                                  uint32_t gamepad_count) const {
    auto it = bindings_.find(action);
    if (it == bindings_.end()) {
        return false;
    }

    for (const auto& binding : it->second) {
        switch (binding.type) {
            case BindingType::Key:
                if (keyboard.was_pressed_this_frame(binding.key)) {
                    return true;
                }
                break;
            case BindingType::MouseButton:
                if (mouse.was_pressed_this_frame(binding.mouse_button)) {
                    return true;
                }
                break;
            case BindingType::GamepadButton:
                if (binding.gamepad_index < gamepad_count) {
                    if (gamepads[binding.gamepad_index].was_pressed_this_frame(binding.gamepad_button)) {
                        return true;
                    }
                }
                break;
            default:
                break;
        }
    }

    return false;
}

bool InputMapping::was_action_released_this_frame(const std::string& action,
                                                   const Keyboard& keyboard,
                                                   const Mouse& mouse,
                                                   const Gamepad* gamepads,
                                                   uint32_t gamepad_count) const {
    auto it = bindings_.find(action);
    if (it == bindings_.end()) {
        return false;
    }

    for (const auto& binding : it->second) {
        switch (binding.type) {
            case BindingType::Key:
                if (keyboard.was_released_this_frame(binding.key)) {
                    return true;
                }
                break;
            case BindingType::MouseButton:
                if (mouse.was_released_this_frame(binding.mouse_button)) {
                    return true;
                }
                break;
            case BindingType::GamepadButton:
                if (binding.gamepad_index < gamepad_count) {
                    if (gamepads[binding.gamepad_index].was_released_this_frame(binding.gamepad_button)) {
                        return true;
                    }
                }
                break;
            default:
                break;
        }
    }

    return false;
}

const std::vector<InputBinding>& InputMapping::get_bindings(const std::string& action) const {
    static const std::vector<InputBinding> empty;
    auto it = bindings_.find(action);
    if (it != bindings_.end()) {
        return it->second;
    }
    return empty;
}

namespace {

// File format (tab-separated, one binding per line):
//   # comment lines start with '#'
//   <action>\tkey\t<key code>
//   <action>\tmouse\t<button code>
//   <action>\tgamepad\t<pad index>\t<button code>
// Enum values are stored numerically so the format is stable across builds.

std::vector<std::string> split_tabs(const std::string& line) {
    std::vector<std::string> parts;
    size_t start = 0;
    while (true) {
        size_t tab = line.find('\t', start);
        if (tab == std::string::npos) {
            parts.push_back(line.substr(start));
            break;
        }
        parts.push_back(line.substr(start, tab - start));
        start = tab + 1;
    }
    return parts;
}

bool parse_int(const std::string& s, long& out) {
    if (s.empty()) {
        return false;
    }
    char* end = nullptr;
    out = std::strtol(s.c_str(), &end, 10);
    return end != s.c_str() && *end == '\0';
}

} // namespace

bool InputMapping::load_from_file(const char* filepath) {
    FILE* file = std::fopen(filepath, "r");
    if (!file) {
        return false;
    }

    bindings_.clear();
    char line[1024];
    bool ok = true;
    while (std::fgets(line, sizeof(line), file)) {
        std::string text(line);
        // Strip trailing newline/CR
        while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
            text.pop_back();
        }
        if (text.empty() || text[0] == '#') {
            continue;
        }

        std::vector<std::string> parts = split_tabs(text);
        if (parts.size() < 3) {
            ok = false;
            continue;
        }

        const std::string& action = parts[0];
        const std::string& kind = parts[1];
        long a = 0, b = 0;

        if (kind == "key" && parts.size() == 3 && parse_int(parts[2], a)) {
            map_action(action, static_cast<KeyCode>(a));
        } else if (kind == "mouse" && parts.size() == 3 && parse_int(parts[2], a)) {
            map_action(action, static_cast<MouseButton>(a));
        } else if (kind == "gamepad" && parts.size() == 4 &&
                   parse_int(parts[2], a) && parse_int(parts[3], b)) {
            map_action(action, static_cast<uint32_t>(a),
                       static_cast<GamepadButton>(b));
        } else {
            ok = false;
        }
    }

    std::fclose(file);
    return ok;
}

bool InputMapping::save_to_file(const char* filepath) const {
    FILE* file = std::fopen(filepath, "w");
    if (!file) {
        return false;
    }

    std::fprintf(file, "# PokoX input mapping\n");
    for (const auto& entry : bindings_) {
        for (const auto& binding : entry.second) {
            switch (binding.type) {
                case BindingType::Key:
                    std::fprintf(file, "%s\tkey\t%u\n", entry.first.c_str(),
                                 static_cast<uint32_t>(binding.key));
                    break;
                case BindingType::MouseButton:
                    std::fprintf(file, "%s\tmouse\t%u\n", entry.first.c_str(),
                                 static_cast<uint32_t>(binding.mouse_button));
                    break;
                case BindingType::GamepadButton:
                    std::fprintf(file, "%s\tgamepad\t%u\t%u\n", entry.first.c_str(),
                                 binding.gamepad_index,
                                 static_cast<uint32_t>(binding.gamepad_button));
                    break;
                default:
                    break;
            }
        }
    }

    std::fclose(file);
    return true;
}

} // namespace poko::input
