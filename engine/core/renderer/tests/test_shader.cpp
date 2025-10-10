#include <gtest/gtest.h>
#include "renderer/shader.hpp"

using namespace PyNovaGE::Renderer;

class ShaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup for shader tests
    }
};

TEST_F(ShaderTest, ShaderCreation) {
    Shader shader;
    EXPECT_FALSE(shader.IsValid()); // Should be invalid without OpenGL context
}

TEST_F(ShaderTest, ShaderLibrarySingleton) {
    ShaderLibrary& lib1 = ShaderLibrary::Instance();
    ShaderLibrary& lib2 = ShaderLibrary::Instance();
    EXPECT_EQ(&lib1, &lib2); // Should be same instance
}