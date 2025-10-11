#include <gtest/gtest.h>
#include "renderer/surface.hpp"

using namespace PyNovaGE::Renderer;

TEST(SurfaceCompile, ConstructDestructOnly) {
    // Constructing and destructing should not crash even without a GL context.
    Surface s;
    // Avoid calling Create() because it requires a valid GL context.
    SUCCEED();
}
