// =============================================================================
// PokoX M0-B — SDL2 window / input feasibility test.
//
// Answers one question: can this toolchain create a resizable Win32 window,
// receive keyboard and mouse events, and shut down cleanly?
//
// Deliberately does NOT create an OpenGL context. In the Diligent test the
// graphics context has exactly one owner (Diligent); here there is none. That
// keeps the two experiments independent, so a failure in one cannot be caused
// by the other.
//
// Automation flags exist because a window that must be closed by hand cannot be
// asserted on in a script:
//   --frames N     exit after N rendered frames (0 = run until closed)
//   --self-test    inject synthetic key/mouse/resize events, verify they arrive
//   --seconds S    exit after S seconds regardless of frame count
// =============================================================================

#define SDL_MAIN_HANDLED

#include <SDL.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

struct Options {
    int frames = 0;      // 0 = unlimited
    double seconds = 0;  // 0 = unlimited
    bool self_test = false;
};

struct EventStats {
    std::uint64_t key_down = 0;
    std::uint64_t key_up = 0;
    std::uint64_t mouse_motion = 0;
    std::uint64_t mouse_button_down = 0;
    std::uint64_t mouse_wheel = 0;
    std::uint64_t window_resized = 0;
    std::uint64_t mouse_focus = 0;
    std::uint64_t quit = 0;
};

void print_usage(const char* exe) {
    std::printf("usage: %s [--frames N] [--seconds S] [--self-test]\n", exe);
}

bool parse_args(int argc, char** argv, Options& options) {
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];

        if (std::strcmp(arg, "--self-test") == 0) {
            options.self_test = true;
            continue;
        }
        if (std::strcmp(arg, "--help") == 0 || std::strcmp(arg, "-h") == 0) {
            print_usage(argv[0]);
            return false;
        }
        if (std::strcmp(arg, "--frames") == 0 && i + 1 < argc) {
            options.frames = std::atoi(argv[++i]);
            continue;
        }
        if (std::strcmp(arg, "--seconds") == 0 && i + 1 < argc) {
            options.seconds = std::atof(argv[++i]);
            continue;
        }

        std::fprintf(stderr, "unknown argument: %s\n", arg);
        print_usage(argv[0]);
        return false;
    }
    return true;
}

// Injects events through the SDL queue so the dispatch code path is exercised
// without a human at the keyboard. This proves handling, not OS delivery; real
// hardware input is verified interactively and recorded in the M0 report.
void inject_synthetic_events(SDL_Window* window) {
    SDL_Event event{};

    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = SDLK_w;
    event.key.keysym.scancode = SDL_SCANCODE_W;
    SDL_PushEvent(&event);

    SDL_zero(event);
    event.type = SDL_KEYUP;
    event.key.keysym.sym = SDLK_w;
    event.key.keysym.scancode = SDL_SCANCODE_W;
    SDL_PushEvent(&event);

    SDL_zero(event);
    event.type = SDL_MOUSEMOTION;
    event.motion.x = 320;
    event.motion.y = 240;
    event.motion.xrel = 4;
    event.motion.yrel = -2;
    SDL_PushEvent(&event);

    SDL_zero(event);
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.clicks = 1;
    event.button.x = 320;
    event.button.y = 240;
    SDL_PushEvent(&event);

    SDL_zero(event);
    event.type = SDL_MOUSEWHEEL;
    event.wheel.y = 1;
    SDL_PushEvent(&event);

    SDL_zero(event);
    event.type = SDL_WINDOWEVENT;
    event.window.event = SDL_WINDOWEVENT_RESIZED;
    event.window.windowID = SDL_GetWindowID(window);
    event.window.data1 = 1024;
    event.window.data2 = 600;
    SDL_PushEvent(&event);
}

void handle_event(const SDL_Event& event, EventStats& stats, bool& running) {
    switch (event.type) {
        case SDL_QUIT:
            ++stats.quit;
            running = false;
            break;

        case SDL_KEYDOWN:
            ++stats.key_down;
            if (event.key.repeat == 0 && event.key.keysym.sym == SDLK_ESCAPE) {
                std::printf("[input] Escape pressed -> requesting shutdown\n");
                running = false;
            }
            break;

        case SDL_KEYUP:
            ++stats.key_up;
            break;

        case SDL_MOUSEMOTION:
            ++stats.mouse_motion;
            break;

        case SDL_MOUSEBUTTONDOWN:
            ++stats.mouse_button_down;
            if (event.button.button == SDL_BUTTON_LEFT) {
                ++stats.mouse_focus;
            }
            break;

        case SDL_MOUSEWHEEL:
            ++stats.mouse_wheel;
            break;

        case SDL_WINDOWEVENT:
            switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED: {
                    ++stats.window_resized;
                    int current_w = 0;
                    int current_h = 0;
                    SDL_GetWindowSize(SDL_GetWindowFromID(event.window.windowID), &current_w,
                                      &current_h);
                    // Report both numbers: the event payload is what the OS
                    // asked for, the queried size is what SDL actually applied.
                    std::printf("[window] resize event %dx%d, window now %dx%d\n",
                                event.window.data1, event.window.data2, current_w, current_h);
                    break;
                }
                case SDL_WINDOWEVENT_CLOSE:
                    std::printf("[window] close requested\n");
                    running = false;
                    break;
                case SDL_WINDOWEVENT_MINIMIZED:
                    std::printf("[window] minimized\n");
                    break;
                case SDL_WINDOWEVENT_RESTORED:
                    std::printf("[window] restored\n");
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    std::printf("[window] focus gained\n");
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    std::printf("[window] focus lost\n");
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }
}

}  // namespace

int main(int argc, char** argv) {
    Options options;
    if (!parse_args(argc, argv, options)) {
        return 2;
    }

    SDL_SetMainReady();

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_version compiled{};
    SDL_VERSION(&compiled);
    SDL_version linked{};
    SDL_GetVersion(&linked);

    std::printf("PokoX M0-B SDL2 window test\n");
    std::printf("  SDL compiled    : %d.%d.%d\n", compiled.major, compiled.minor,
                compiled.patch);
    std::printf("  SDL linked      : %d.%d.%d\n", linked.major, linked.minor,
                linked.patch);
    std::printf("  video driver    : %s\n",
                SDL_GetCurrentVideoDriver() ? SDL_GetCurrentVideoDriver() : "(none)");
    std::printf("  displays        : %d\n", SDL_GetNumVideoDisplays());

    SDL_Window* window = SDL_CreateWindow(
        "PokoX M0-B SDL2 feasibility",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280,
        720,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    int w = 0;
    int h = 0;
    SDL_GetWindowSize(window, &w, &h);
    std::printf("  window          : %dx%d resizable\n", w, h);

    EventStats stats;

    // Flush the events that window creation itself produces (focus, the first
    // mouse position, a pending resize) so the self-test below can attribute
    // counts to the injected events only.
    if (options.self_test) {
        const std::uint32_t flush_until = SDL_GetTicks() + 150;
        bool flushing = true;
        while (flushing && SDL_GetTicks() < flush_until) {
            SDL_Event event;
            while (SDL_PollEvent(&event) != 0) {
                handle_event(event, stats, flushing);
            }
            SDL_Delay(10);
        }
        stats = EventStats{};
        inject_synthetic_events(window);
        std::printf("  self-test       : injected synthetic input events\n");
    }

    std::printf("  mode            : %s\n",
                options.frames > 0 ? "automated" : "interactive (Esc or close to quit)");

    bool running = true;
    std::uint64_t frame = 0;
    const std::uint32_t start_ticks = SDL_GetTicks();

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            handle_event(event, stats, running);
        }

        if (!running) {
            break;
        }

        // Nothing is rendered in this test; the sleep keeps it honest about the
        // fact that a real frame loop would live here.
        SDL_Delay(16);
        ++frame;

        if (options.frames > 0 && frame >= static_cast<std::uint64_t>(options.frames)) {
            break;
        }
        if (options.seconds > 0.0 &&
            (SDL_GetTicks() - start_ticks) >= static_cast<std::uint32_t>(options.seconds * 1000.0)) {
            break;
        }
    }

    std::printf("  frames          : %llu\n", static_cast<unsigned long long>(frame));
    std::printf("  events          : keydown=%llu keyup=%llu motion=%llu "
                "mousedown=%llu wheel=%llu resize=%llu quit=%llu\n",
                static_cast<unsigned long long>(stats.key_down),
                static_cast<unsigned long long>(stats.key_up),
                static_cast<unsigned long long>(stats.mouse_motion),
                static_cast<unsigned long long>(stats.mouse_button_down),
                static_cast<unsigned long long>(stats.mouse_wheel),
                static_cast<unsigned long long>(stats.window_resized),
                static_cast<unsigned long long>(stats.quit));

    SDL_DestroyWindow(window);
    SDL_Quit();

    int exit_code = 0;
    if (options.self_test) {
        const bool ok = stats.key_down == 1 && stats.key_up == 1 &&
                        stats.mouse_motion == 1 && stats.mouse_button_down == 1 &&
                        stats.mouse_wheel == 1 && stats.window_resized == 1;
        if (!ok) {
            std::printf("FAIL: expected exactly one of each synthetic event\n");
            exit_code = 1;
        }
    }

    std::printf("%s\n", exit_code == 0 ? "PASS" : "FAILED");
    return exit_code;
}
