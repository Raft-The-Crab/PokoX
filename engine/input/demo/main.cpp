/**
 * @file main.cpp
 * @brief Input Module Demo
 *
 * Demonstrates keyboard, mouse, and gamepad input handling.
 * Visualizes input state in real-time.
 */

#define SDL_MAIN_HANDLED

#include "poko/input/input.h"

#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char** argv) {
    std::printf("PokoX Input Module Demo\n");
    std::printf("========================\n\n");

    uint64_t max_frames = 0;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            max_frames = static_cast<uint64_t>(std::atoi(argv[++i]));
        }
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "PokoX Input Demo",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280,
        720,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Initialize input manager
    poko::input::InputManager input;
    if (!input.initialize()) {
        std::fprintf(stderr, "Failed to initialize input manager\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::printf("Input manager initialized\n");
    std::printf("Press ESC to exit\n\n");

    // Main loop
    bool running = true;
    uint64_t frame_count = 0;

    while (running) {
        // Update input
        input.update();

        // Check for exit
        if (input.keyboard().is_pressed(poko::input::KeyCode::Escape)) {
            running = false;
        }

        // Print input state every 60 frames (approx 1 second at 60 FPS)
        if (frame_count % 60 == 0) {
            std::printf("\n--- Frame %llu ---\n", frame_count);

            // Keyboard state
            std::printf("Keyboard:\n");
            if (input.keyboard().is_pressed(poko::input::KeyCode::W)) std::printf("  W: pressed\n");
            if (input.keyboard().is_pressed(poko::input::KeyCode::A)) std::printf("  A: pressed\n");
            if (input.keyboard().is_pressed(poko::input::KeyCode::S)) std::printf("  S: pressed\n");
            if (input.keyboard().is_pressed(poko::input::KeyCode::D)) std::printf("  D: pressed\n");
            if (input.keyboard().is_pressed(poko::input::KeyCode::Space)) std::printf("  Space: pressed\n");

            auto modifiers = input.keyboard().modifiers();
            if (poko::input::has_modifier(modifiers, poko::input::KeyModifier::Shift)) {
                std::printf("  Shift: held\n");
            }
            if (poko::input::has_modifier(modifiers, poko::input::KeyModifier::Control)) {
                std::printf("  Control: held\n");
            }

            // Mouse state
            std::printf("Mouse:\n");
            auto mouse_pos = input.mouse().position();
            std::printf("  Position: (%.1f, %.1f)\n", mouse_pos.x, mouse_pos.y);
            if (input.mouse().is_pressed(poko::input::MouseButton::Left)) {
                std::printf("  Left button: pressed\n");
            }
            if (input.mouse().is_pressed(poko::input::MouseButton::Right)) {
                std::printf("  Right button: pressed\n");
            }
            auto scroll = input.mouse().scroll_delta();
            if (scroll.y != 0.0f) {
                std::printf("  Scroll: %.1f\n", scroll.y);
            }

            // Gamepad state
            for (uint32_t i = 0; i < 4; ++i) {
                auto& gamepad = input.gamepad(i);
                if (gamepad.is_connected()) {
                    std::printf("Gamepad %u:\n", i);
                    if (gamepad.is_pressed(poko::input::GamepadButton::A)) {
                        std::printf("  A: pressed\n");
                    }
                    if (gamepad.is_pressed(poko::input::GamepadButton::B)) {
                        std::printf("  B: pressed\n");
                    }
                    if (gamepad.is_pressed(poko::input::GamepadButton::X)) {
                        std::printf("  X: pressed\n");
                    }
                    if (gamepad.is_pressed(poko::input::GamepadButton::Y)) {
                        std::printf("  Y: pressed\n");
                    }

                    auto left_stick = gamepad.stick_axis(poko::input::GamepadAxis::LeftStick);
                    if (left_stick.length() > 0.1f) {
                        std::printf("  Left stick: (%.2f, %.2f)\n", left_stick.x, left_stick.y);
                    }

                    auto right_stick = gamepad.stick_axis(poko::input::GamepadAxis::RightStick);
                    if (right_stick.length() > 0.1f) {
                        std::printf("  Right stick: (%.2f, %.2f)\n", right_stick.x, right_stick.y);
                    }
                }
            }
        }

        // Reset frame state
        input.reset_frame_state();

        // Present (platform handles this)
        SDL_Delay(16);  // ~60 FPS

        frame_count++;

        if (max_frames > 0 && frame_count >= max_frames) {
            running = false;
        }
    }

    input.shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::printf("\nDemo finished successfully\n");
    return 0;
}
