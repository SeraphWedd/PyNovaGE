#include <gtest/gtest.h>
#include "renderer/sprite_renderer.hpp"

using namespace PyNovaGE::Renderer;

class SpriteRendererTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup for sprite renderer tests
    }
};

TEST_F(SpriteRendererTest, SpriteRendererCreation) {
    // This test just ensures we can create the object
    SpriteRenderer renderer;
    // Basic creation test - more functionality tests would come later
    SUCCEED();
}