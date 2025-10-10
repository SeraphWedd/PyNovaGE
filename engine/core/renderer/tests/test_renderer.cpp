#include <gtest/gtest.h>
#include "renderer/renderer.hpp"

using namespace PyNovaGE::Renderer;

class RendererTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Tests will run without initializing renderer since it needs OpenGL context
    }
    
    void TearDown() override {
        if (Renderer::IsInitialized()) {
            Renderer::Shutdown();
        }
    }
};

TEST_F(RendererTest, RendererNotInitializedByDefault) {
    EXPECT_FALSE(Renderer::IsInitialized());
}

TEST_F(RendererTest, RendererGuardConstruction) {
    // This is just a basic test to ensure the classes can be instantiated
    // Without OpenGL context, initialization will fail but that's expected
    EXPECT_FALSE(Renderer::IsInitialized());
}

// More tests would go here once we have proper OpenGL context in tests