#include <gtest/gtest.h>
#include "../../include/lighting/spot_light.hpp"

using namespace pynovage::math::lighting;

class SpotLightTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup will go here
    }

    // Common test variables will go here
    const float epsilon = 1e-6f;
};

// Basic tests will go here