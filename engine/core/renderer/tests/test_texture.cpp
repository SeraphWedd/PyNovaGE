#include <gtest/gtest.h>
#include "renderer/texture.hpp"

using namespace PyNovaGE::Renderer;

class TextureTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup for texture tests
    }
};

TEST_F(TextureTest, TextureCreation) {
    Texture texture;
    EXPECT_FALSE(texture.IsValid()); // Should be invalid without OpenGL context
}

TEST_F(TextureTest, TextureManagerSingleton) {
    TextureManager& mgr1 = TextureManager::Instance();
    TextureManager& mgr2 = TextureManager::Instance();
    EXPECT_EQ(&mgr1, &mgr2); // Should be same instance
}