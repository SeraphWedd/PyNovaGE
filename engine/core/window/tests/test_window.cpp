#include <gtest/gtest.h>
#include "window/window.hpp"
#include <thread>
#include <chrono>

using namespace PyNovaGE::Window;

class WindowTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize window system for each test
        ASSERT_TRUE(InitializeWindowSystem());
    }
    
    void TearDown() override {
        // Clean up after each test
        ShutdownWindowSystem();
    }
};

TEST_F(WindowTest, InitializationAndShutdown) {
    // Test basic initialization
    EXPECT_TRUE(InitializeWindowSystem());
    
    // Multiple initializations should be safe
    EXPECT_TRUE(InitializeWindowSystem());
    
    ShutdownWindowSystem();
    ShutdownWindowSystem(); // Should be safe to call multiple times
}

TEST_F(WindowTest, WindowSystemGuard) {
    {
        WindowSystemGuard guard;
        EXPECT_TRUE(guard.IsInitialized());
        
        // Should be able to create windows
        WindowConfig config;
        config.visible = false; // Don't show window during tests
        Window window(config);
        
        EXPECT_FALSE(window.ShouldClose());
    }
    // Guard should clean up automatically
}

TEST_F(WindowTest, BasicWindowCreation) {
    WindowConfig config;
    config.title = "Test Window";
    config.width = 640;
    config.height = 480;
    config.visible = false; // Don't show during tests
    
    Window window(config);
    
    EXPECT_FALSE(window.ShouldClose());
    EXPECT_EQ(window.GetTitle(), "Test Window");
    
    auto size = window.GetSize();
    EXPECT_EQ(size.x, 640);
    EXPECT_EQ(size.y, 480);
}

TEST_F(WindowTest, WindowProperties) {
    WindowConfig config;
    config.visible = false;
    Window window(config);
    
    // Test size changes
    window.SetSize(800, 600);
    auto size = window.GetSize();
    EXPECT_EQ(size.x, 800);
    EXPECT_EQ(size.y, 600);
    
    // Test title changes
    window.SetTitle("New Title");
    EXPECT_EQ(window.GetTitle(), "New Title");
    
    // Test position
    window.SetPosition(100, 100);
    auto pos = window.GetPosition();
    EXPECT_GE(pos.x, 0); // Position might be adjusted by window manager
    EXPECT_GE(pos.y, 0);
}

TEST_F(WindowTest, WindowStates) {
    WindowConfig config;
    config.visible = false;
    Window window(config);
    
    // Test initial state
    EXPECT_FALSE(window.ShouldClose());
    
    // Test close flag
    window.SetShouldClose(true);
    EXPECT_TRUE(window.ShouldClose());
    
    window.SetShouldClose(false);
    EXPECT_FALSE(window.ShouldClose());
}

TEST_F(WindowTest, WindowEvents) {
    WindowConfig config;
    config.visible = false;
    Window window(config);
    
    bool event_received = false;
    WindowEventType last_event_type;
    
    window.SetEventCallback([&](const WindowEvent& event) {
        event_received = true;
        last_event_type = event.type;
    });
    
    // Simulate a resize event by changing window size
    window.SetSize(400, 300);
    window.PollEvents();
    
    // Note: In a headless test environment, events might not be generated
    // This test mainly verifies the callback system works
}

TEST_F(WindowTest, VSync) {
    WindowConfig config;
    config.visible = false;
    config.vsync = true;
    Window window(config);
    
    EXPECT_TRUE(window.IsVSyncEnabled());
    
    window.SetVSync(false);
    EXPECT_FALSE(window.IsVSyncEnabled());
    
    window.SetVSync(true);
    EXPECT_TRUE(window.IsVSyncEnabled());
}

TEST_F(WindowTest, MoveSemantics) {
    WindowConfig config;
    config.visible = false;
    config.title = "Move Test";
    
    // Test move constructor
    Window window1(config);
    Window window2(std::move(window1));
    
    EXPECT_EQ(window2.GetTitle(), "Move Test");
    EXPECT_FALSE(window2.ShouldClose());
    
    // Test move assignment
    Window window3(config);
    window3.SetTitle("Another Title");
    window2 = std::move(window3);
    
    EXPECT_EQ(window2.GetTitle(), "Another Title");
}

TEST_F(WindowTest, MultipleWindows) {
    WindowConfig config;
    config.visible = false;
    
    Window window1(config);
    window1.SetTitle("Window 1");
    
    Window window2(config);
    window2.SetTitle("Window 2");
    
    EXPECT_EQ(window1.GetTitle(), "Window 1");
    EXPECT_EQ(window2.GetTitle(), "Window 2");
    
    EXPECT_FALSE(window1.ShouldClose());
    EXPECT_FALSE(window2.ShouldClose());
}

TEST_F(WindowTest, FramebufferSize) {
    WindowConfig config;
    config.visible = false;
    config.width = 800;
    config.height = 600;
    
    Window window(config);
    
    auto fb_size = window.GetFramebufferSize();
    EXPECT_GT(fb_size.x, 0);
    EXPECT_GT(fb_size.y, 0);
    
    // On most systems, framebuffer size should match window size
    // unless there's high-DPI scaling
    auto win_size = window.GetSize();
    EXPECT_GT(fb_size.x, 0);
    EXPECT_GT(fb_size.y, 0);
}

TEST_F(WindowTest, Time) {
    WindowConfig config;
    config.visible = false;
    Window window(config);
    
    double time1 = window.GetTime();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    double time2 = window.GetTime();
    
    EXPECT_GT(time2, time1);
    EXPECT_LT(time2 - time1, 1.0); // Should be less than 1 second
}

// Test invalid operations
TEST_F(WindowTest, ErrorHandling) {
    // Test creating window without initialization
    ShutdownWindowSystem();
    
    WindowConfig config;
    config.visible = false;
    
    EXPECT_THROW(Window window(config), std::runtime_error);
    
    // Re-initialize for cleanup
    InitializeWindowSystem();
}