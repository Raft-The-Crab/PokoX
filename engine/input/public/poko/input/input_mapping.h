/**
 * @file input_mapping.h
 * @brief Input remapping and action mapping.
 *
 * Allows users to customize input bindings by mapping physical keys/buttons
 * to logical game actions. Supports keyboard, mouse, and gamepad.
 *
 * Usage example:
 * @code
 * InputMapping mapping;
 * mapping.map_action("jump", KeyCode::Space);
 * mapping.map_action("jump", GamepadButton::A);
 * mapping.map_action("fire", MouseButton::Left);
 *
 * if (mapping.is_action_pressed("jump")) {
 *     // Jump action triggered
 * }
 * @endcode
 */

#ifndef POKO_INPUT_INPUT_MAPPING_H
#define POKO_INPUT_INPUT_MAPPING_H

#include "key_codes.h"
#include "keyboard.h"
#include "mouse.h"
#include "gamepad.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace poko::input {

/**
 * @brief Input binding type.
 */
enum class BindingType : uint32_t {
    None = 0,
    Key = 1,
    MouseButton = 2,
    GamepadButton = 3
};

/**
 * @brief Input binding for a single action.
 */
struct InputBinding {
    BindingType type = BindingType::None;
    KeyCode key = KeyCode::Unknown;
    MouseButton mouse_button = MouseButton::Unknown;
    uint32_t gamepad_index = 0;
    GamepadButton gamepad_button = GamepadButton::Unknown;
};

/**
 * @brief Input mapping system.
 *
 * Maps logical action names to physical input bindings.
 * Supports multiple bindings per action.
 */
class InputMapping {
public:
    InputMapping();
    ~InputMapping() = default;

    /**
     * @brief Maps a keyboard key to an action.
     *
     * @param action Action name (e.g., "jump", "fire").
     * @param key Key to bind.
     */
    void map_action(const std::string& action, KeyCode key);

    /**
     * @brief Maps a mouse button to an action.
     *
     * @param action Action name.
     * @param button Mouse button to bind.
     */
    void map_action(const std::string& action, MouseButton button);

    /**
     * @brief Maps a gamepad button to an action.
     *
     * @param action Action name.
     * @param gamepad_index Gamepad player index.
     * @param button Gamepad button to bind.
     */
    void map_action(const std::string& action, uint32_t gamepad_index, GamepadButton button);

    /**
     * @brief Removes all bindings for an action.
     *
     * @param action Action name.
     */
    void unmap_action(const std::string& action);

    /**
     * @brief Removes a specific binding for an action.
     *
     * @param action Action name.
     * @param binding Binding to remove.
     */
    void unmap_binding(const std::string& action, const InputBinding& binding);

    /**
     * @brief Returns true if an action is currently pressed.
     *
     * @param action Action name.
     * @param keyboard Keyboard state.
     * @param mouse Mouse state.
     * @param gamepads Array of gamepad states.
     * @param gamepad_count Number of gamepads.
     * @return true if any binding for this action is pressed.
     */
    bool is_action_pressed(const std::string& action,
                          const Keyboard& keyboard,
                          const Mouse& mouse,
                          const Gamepad* gamepads,
                          uint32_t gamepad_count) const;

    /**
     * @brief Returns true if an action was pressed this frame.
     *
     * @param action Action name.
     * @param keyboard Keyboard state.
     * @param mouse Mouse state.
     * @param gamepads Array of gamepad states.
     * @param gamepad_count Number of gamepads.
     * @return true if any binding for this action was pressed this frame.
     */
    bool was_action_pressed_this_frame(const std::string& action,
                                        const Keyboard& keyboard,
                                        const Mouse& mouse,
                                        const Gamepad* gamepads,
                                        uint32_t gamepad_count) const;

    /**
     * @brief Returns true if an action was released this frame.
     *
     * @param action Action name.
     * @param keyboard Keyboard state.
     * @param mouse Mouse state.
     * @param gamepads Array of gamepad states.
     * @param gamepad_count Number of gamepads.
     * @return true if any binding for this action was released this frame.
     */
    bool was_action_released_this_frame(const std::string& action,
                                       const Keyboard& keyboard,
                                       const Mouse& mouse,
                                       const Gamepad* gamepads,
                                       uint32_t gamepad_count) const;

    /**
     * @brief Returns all bindings for an action.
     *
     * @param action Action name.
     * @return Vector of bindings.
     */
    const std::vector<InputBinding>& get_bindings(const std::string& action) const;

    /**
     * @brief Loads input mappings from a file (simplified placeholder).
     *
     * @param filepath Path to mapping file.
     * @return true on success.
     */
    bool load_from_file(const char* filepath);

    /**
     * @brief Saves input mappings to a file (simplified placeholder).
     *
     * @param filepath Path to mapping file.
     * @return true on success.
     */
    bool save_to_file(const char* filepath) const;

private:
    std::unordered_map<std::string, std::vector<InputBinding>> bindings_;
};

} // namespace poko::input

#endif // POKO_INPUT_INPUT_MAPPING_H
