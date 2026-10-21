#include "poko/platform/WindowsWindow.hpp"
#include "poko/core/Logger.hpp"
#include <iostream>

namespace test {

void test_window_creation() {
    poko::core::Logger::instance().info("Testing window creation");
    
    poko::platform::WindowConfig config;
    config.title = "PokoX Test Window";
    config.width = 800;
    config.height = 600;
    config.resizable = true;
    
    poko::platform::WindowsWindow window(config);
    
    if (!window.is_valid()) {
        std::cerr << "Window creation failed\n";
        return;
    }
    
    poko::core::Logger::instance().info("Window created successfully");
}

void test_window_dimensions() {
    poko::core::Logger::instance().info("Testing window dimensions");
    
    poko::platform::WindowConfig config;
    config.title = "PokoX Test Window";
    config.width = 1024;
    config.height = 768;
    
    poko::platform::WindowsWindow window(config);
    
    if (!window.is_valid()) {
        std::cerr << "Window creation failed\n";
        return;
    }
    
    if (window.width() != 1024 || window.height() != 768) {
        std::cerr << "Window dimensions mismatch\n";
        return;
    }
    
    poko::core::Logger::instance().info("Window dimensions correct");
}

void test_resize_callback() {
    poko::core::Logger::instance().info("Testing resize callback");
    
    poko::platform::WindowConfig config;
    config.title = "PokoX Test Window";
    config.width = 800;
    config.height = 600;
    
    poko::platform::WindowsWindow window(config);
    
    if (!window.is_valid()) {
        std::cerr << "Window creation failed\n";
        return;
    }
    
    bool callback_called = false;
    window.set_resize_callback([&callback_called](int width, int height) {
        callback_called = true;
        poko::core::Logger::instance().info("Resize callback called: " + std::to_string(width) + "x" + std::to_string(height));
    });
    
    poko::core::Logger::instance().info("Resize callback set");
}

} // namespace test

int main() {
    try {
        poko::core::Logger::instance().info("=== Platform Tests Start ===");
        
        test::test_window_creation();
        test::test_window_dimensions();
        test::test_resize_callback();
        
        poko::core::Logger::instance().info("=== Platform Tests End ===");
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test exception: " << e.what() << "\n";
        return 1;
    }
}
