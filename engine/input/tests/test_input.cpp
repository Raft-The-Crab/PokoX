/**
 * @file test_input.cpp
 * @brief Unit tests for PokoX input library.
 */

#include "poko/input/input.h"

#include <cstdio>
#include <cassert>

int main() {
    std::printf("PokoX Input Library Tests\n");
    std::printf("=========================\n\n");

    // KeyCode tests
    {
        std::printf("KeyCode tests...\n");
        static_assert(static_cast<uint32_t>(poko::input::KeyCode::A) == 1);
        static_assert(static_cast<uint32_t>(poko::input::KeyCode::Z) == 26);
        static_assert(static_cast<uint32_t>(poko::input::KeyCode::Escape) == 73);
        std::printf("  PASS\n");
    }

    // KeyModifier tests
    {
        std::printf("KeyModifier tests...\n");
        using namespace poko::input;

        KeyModifier flags = KeyModifier::Shift | KeyModifier::Control;
        assert(has_modifier(flags, KeyModifier::Shift));
        assert(has_modifier(flags, KeyModifier::Control));
        assert(!has_modifier(flags, KeyModifier::Alt));

        flags = flags | KeyModifier::Alt;
        assert(has_modifier(flags, KeyModifier::Alt));

        flags = flags & ~KeyModifier::Shift;
        assert(!has_modifier(flags, KeyModifier::Shift));
        assert(has_modifier(flags, KeyModifier::Control));

        std::printf("  PASS\n");
    }

    // Keyboard construction
    {
        std::printf("Keyboard construction tests...\n");
        poko::input::Keyboard keyboard;
        assert(!keyboard.is_pressed(poko::input::KeyCode::A));
        assert(!keyboard.was_pressed_this_frame(poko::input::KeyCode::Space));
        assert(keyboard.modifiers() == poko::input::KeyModifier::None);
        std::printf("  PASS\n");
    }

    // Mouse construction
    {
        std::printf("Mouse construction tests...\n");
        poko::input::Mouse mouse;
        assert(!mouse.is_pressed(poko::input::MouseButton::Left));
        assert(mouse.position().x == 0.0f);
        assert(mouse.position().y == 0.0f);
        assert(mouse.scroll_delta().x == 0.0f);
        assert(mouse.scroll_delta().y == 0.0f);
        std::printf("  PASS\n");
    }

    // Gamepad construction
    {
        std::printf("Gamepad construction tests...\n");
        poko::input::Gamepad gamepad(0);
        assert(!gamepad.is_connected());
        assert(!gamepad.is_pressed(poko::input::GamepadButton::A));
        assert(gamepad.axis(poko::input::GamepadAxis::LeftStickX) == 0.0f);
        std::printf("  PASS\n");
    }

    // Gamepad configuration
    {
        std::printf("Gamepad configuration tests...\n");
        poko::input::Gamepad gamepad(0);
        poko::input::Gamepad::Config config;
        config.stick_deadzone = 0.2f;
        config.trigger_deadzone = 0.1f;
        gamepad.set_config(config);
        std::printf("  PASS\n");
    }

    // InputManager construction
    {
        std::printf("InputManager construction tests...\n");
        poko::input::InputManager input;
        assert(!input.keyboard().is_pressed(poko::input::KeyCode::W));
        assert(!input.mouse().is_pressed(poko::input::MouseButton::Left));
        assert(!input.gamepad(0).is_connected());
        std::printf("  PASS\n");
    }

    // InputManager gamepad access
    {
        std::printf("InputManager gamepad access tests...\n");
        poko::input::InputManager input;
        for (uint32_t i = 0; i < 4; ++i) {
            assert(!input.gamepad(i).is_connected());
        }
        // Out of bounds should fallback to gamepad 0
        assert(!input.gamepad(10).is_connected());
        std::printf("  PASS\n");
    }

    std::printf("\nAll input library tests passed.\n");
    return 0;
}
