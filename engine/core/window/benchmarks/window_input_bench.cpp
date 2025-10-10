#include <benchmark/benchmark.h>
#include "window/window.hpp"
#include "window/input.hpp"
#include <memory>

using namespace PyNovaGE::Window;

// Global window system guard
static std::unique_ptr<WindowSystemGuard> g_window_guard;

// Setup function for benchmarks
static void DoSetup(const benchmark::State& /*state*/) {
    if (!g_window_guard) {
        g_window_guard = std::make_unique<WindowSystemGuard>();
    }
}

// Cleanup function for benchmarks
static void DoTeardown(const benchmark::State& /*state*/) {
    // Keep the guard alive for all benchmarks
}

// Window creation benchmark
static void BM_WindowCreation(benchmark::State& state) {
    DoSetup(state);
    
    WindowConfig config;
    config.visible = false; // Don't show windows during benchmarks
    config.width = 800;
    config.height = 600;
    
    for (auto _ : state) {
        Window window(config);
        benchmark::DoNotOptimize(window);
    }
    
    DoTeardown(state);
}
BENCHMARK(BM_WindowCreation);

// Window property access benchmark
static void BM_WindowPropertyAccess(benchmark::State& state) {
    DoSetup(state);
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    
    for (auto _ : state) {
        auto size = window.GetSize();
        auto pos = window.GetPosition();
        auto title = window.GetTitle();
        bool should_close = window.ShouldClose();
        bool is_focused = window.IsFocused();
        
        benchmark::DoNotOptimize(size);
        benchmark::DoNotOptimize(pos);
        benchmark::DoNotOptimize(title);
        benchmark::DoNotOptimize(should_close);
        benchmark::DoNotOptimize(is_focused);
    }
    
    DoTeardown(state);
}
BENCHMARK(BM_WindowPropertyAccess);

// Window property modification benchmark
static void BM_WindowPropertyModification(benchmark::State& state) {
    DoSetup(state);
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    
    int counter = 0;
    for (auto _ : state) {
        window.SetSize(800 + (counter % 100), 600 + (counter % 100));
        window.SetPosition(100 + (counter % 50), 100 + (counter % 50));
        window.SetTitle("Benchmark " + std::to_string(counter));
        window.SetShouldClose(counter % 2 == 0);
        counter++;
    }
    
    DoTeardown(state);
}
BENCHMARK(BM_WindowPropertyModification);

// Event polling benchmark
static void BM_EventPolling(benchmark::State& state) {
    DoSetup(state);
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    
    for (auto _ : state) {
        window.PollEvents();
    }
    
    DoTeardown(state);
}
BENCHMARK(BM_EventPolling);

// Input manager creation benchmark
static void BM_InputManagerCreation(benchmark::State& state) {
    DoSetup(state);
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    
    for (auto _ : state) {
        InputManager input(window.GetNativeWindow());
        benchmark::DoNotOptimize(input);
    }
    
    DoTeardown(state);
}
BENCHMARK(BM_InputManagerCreation);

// Input state polling benchmark
static void BM_InputStatePolling(benchmark::State& state) {
    DoSetup(state);
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    InputManager input(window.GetNativeWindow());
    
    for (auto _ : state) {
        // Poll common keys
        bool w = input.IsKeyPressed(Key::W);
        bool a = input.IsKeyPressed(Key::A);
        bool s = input.IsKeyPressed(Key::S);
        bool d = input.IsKeyPressed(Key::D);
        bool space = input.IsKeyPressed(Key::Space);
        bool shift = input.IsKeyPressed(Key::LeftShift);
        
        // Poll mouse buttons
        bool left_mouse = input.IsMouseButtonPressed(MouseButton::Left);
        bool right_mouse = input.IsMouseButtonPressed(MouseButton::Right);
        bool middle_mouse = input.IsMouseButtonPressed(MouseButton::Middle);
        
        benchmark::DoNotOptimize(w);
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(s);
        benchmark::DoNotOptimize(d);
        benchmark::DoNotOptimize(space);
        benchmark::DoNotOptimize(shift);
        benchmark::DoNotOptimize(left_mouse);
        benchmark::DoNotOptimize(right_mouse);
        benchmark::DoNotOptimize(middle_mouse);
    }
    
    DoTeardown(state);
}
BENCHMARK(BM_InputStatePolling);

// Input state "just pressed" checks benchmark
static void BM_InputJustPressedChecks(benchmark::State& state) {
    DoSetup(state);
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    InputManager input(window.GetNativeWindow());
    
    for (auto _ : state) {
        // Check just pressed states
        bool w = input.IsKeyJustPressed(Key::W);
        bool a = input.IsKeyJustPressed(Key::A);
        bool s = input.IsKeyJustPressed(Key::S);
        bool d = input.IsKeyJustPressed(Key::D);
        bool space = input.IsKeyJustPressed(Key::Space);
        
        bool left_mouse = input.IsMouseButtonJustPressed(MouseButton::Left);
        bool right_mouse = input.IsMouseButtonJustPressed(MouseButton::Right);
        
        benchmark::DoNotOptimize(w);
        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(s);
        benchmark::DoNotOptimize(d);
        benchmark::DoNotOptimize(space);
        benchmark::DoNotOptimize(left_mouse);
        benchmark::DoNotOptimize(right_mouse);
    }
    
    DoTeardown(state);
}
BENCHMARK(BM_InputJustPressedChecks);

// Full input update benchmark
static void BM_InputUpdate(benchmark::State& state) {
    DoSetup(state);
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    InputManager input(window.GetNativeWindow());
    
    for (auto _ : state) {
        input.Update();
    }
    
    DoTeardown(state);
}
BENCHMARK(BM_InputUpdate);

// Mouse position and delta access benchmark
static void BM_MousePositionAccess(benchmark::State& state) {
    DoSetup(state);
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    InputManager input(window.GetNativeWindow());
    
    for (auto _ : state) {
        auto pos = input.GetMousePosition();
        auto delta = input.GetMouseDelta();
        auto scroll = input.GetScrollDelta();
        
        benchmark::DoNotOptimize(pos);
        benchmark::DoNotOptimize(delta);
        benchmark::DoNotOptimize(scroll);
    }
    
    DoTeardown(state);
}
BENCHMARK(BM_MousePositionAccess);

// Gamepad state polling benchmark
static void BM_GamepadStatePolling(benchmark::State& state) {
    DoSetup(state);
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    InputManager input(window.GetNativeWindow());
    
    for (auto _ : state) {
        // Check multiple gamepads
        for (int i = 0; i < 4; ++i) {
            bool connected = input.IsGamepadConnected(i);
            bool button_a = input.IsGamepadButtonPressed(i, GamepadButton::A);
            bool button_b = input.IsGamepadButtonPressed(i, GamepadButton::B);
            float left_x = input.GetGamepadAxis(i, GamepadAxis::LeftX);
            float left_y = input.GetGamepadAxis(i, GamepadAxis::LeftY);
            
            benchmark::DoNotOptimize(connected);
            benchmark::DoNotOptimize(button_a);
            benchmark::DoNotOptimize(button_b);
            benchmark::DoNotOptimize(left_x);
            benchmark::DoNotOptimize(left_y);
        }
    }
    
    DoTeardown(state);
}
BENCHMARK(BM_GamepadStatePolling);

// Combined window and input operations benchmark
static void BM_CombinedWindowInput(benchmark::State& state) {
    DoSetup(state);
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    InputManager input(window.GetNativeWindow());
    
    for (auto _ : state) {
        // Window operations
        window.PollEvents();
        auto size = window.GetSize();
        bool should_close = window.ShouldClose();
        
        // Input operations
        input.Update();
        bool w_pressed = input.IsKeyPressed(Key::W);
        bool mouse_pressed = input.IsMouseButtonPressed(MouseButton::Left);
        auto mouse_pos = input.GetMousePosition();
        
        benchmark::DoNotOptimize(size);
        benchmark::DoNotOptimize(should_close);
        benchmark::DoNotOptimize(w_pressed);
        benchmark::DoNotOptimize(mouse_pressed);
        benchmark::DoNotOptimize(mouse_pos);
    }
    
    DoTeardown(state);
}
BENCHMARK(BM_CombinedWindowInput);

// Event callback overhead benchmark
static void BM_EventCallbackOverhead(benchmark::State& state) {
    DoSetup(state);
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    InputManager input(window.GetNativeWindow());
    
    int callback_count = 0;
    window.SetEventCallback([&](const WindowEvent& event) {
        callback_count++;
        benchmark::DoNotOptimize(event);
    });
    
    input.SetInputCallback([&](const InputEvent& event) {
        callback_count++;
        benchmark::DoNotOptimize(event);
    });
    
    for (auto _ : state) {
        // These operations might trigger callbacks
        window.PollEvents();
        input.Update();
        
        benchmark::DoNotOptimize(callback_count);
    }
    
    DoTeardown(state);
}
BENCHMARK(BM_EventCallbackOverhead);

// Stress test: Multiple key checks per frame
static void BM_MassiveInputPolling(benchmark::State& state) {
    DoSetup(state);
    
    WindowConfig config;
    config.visible = false;
    Window window(config);
    InputManager input(window.GetNativeWindow());
    
    // Array of keys to test (common game keys)
    std::vector<Key> keys = {
        Key::W, Key::A, Key::S, Key::D, Key::Q, Key::E, Key::R, Key::T,
        Key::Space, Key::LeftShift, Key::LeftControl, Key::Tab,
        Key::Num1, Key::Num2, Key::Num3, Key::Num4, Key::Num5,
        Key::F1, Key::F2, Key::F3, Key::F4, Key::Escape
    };
    
    for (auto _ : state) {
        for (const auto& key : keys) {
            bool pressed = input.IsKeyPressed(key);
            bool just_pressed = input.IsKeyJustPressed(key);
            bool just_released = input.IsKeyJustReleased(key);
            
            benchmark::DoNotOptimize(pressed);
            benchmark::DoNotOptimize(just_pressed);
            benchmark::DoNotOptimize(just_released);
        }
    }
    
    DoTeardown(state);
}
BENCHMARK(BM_MassiveInputPolling);

BENCHMARK_MAIN();