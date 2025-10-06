#include <gtest/gtest.h>
#include "vector3.hpp"
#include "lighting/directional_light.hpp"
#include "lighting/light_types.hpp"

using namespace pynovage::math;
using namespace pynovage::math::lighting;

class DirectionalLightTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup will go here
    }

    // Common test variables
    const float epsilon = 1e-6f;
};

TEST_F(DirectionalLightTest, DefaultConstructor) {
    DirectionalLight light;
    
    // Should point straight down by default
    EXPECT_NEAR(light.direction.x, 0.0f, epsilon);
    EXPECT_NEAR(light.direction.y, -1.0f, epsilon);
    EXPECT_NEAR(light.direction.z, 0.0f, epsilon);
    
    // Should be set up as a directional light
    EXPECT_EQ(light.type, LightType::Directional);
    EXPECT_EQ(light.features, LightFeatures::CastShadows);
    EXPECT_EQ(light.attenuationModel, AttenuationModel::None);
}

TEST_F(DirectionalLightTest, DirectionConstructor) {
    Vector3 dir(1.0f, 1.0f, 1.0f);
    DirectionalLight light(dir);
    
    // Direction should be normalized
    Vector3 expected = dir.normalized();
    EXPECT_NEAR(light.direction.x, expected.x, epsilon);
    EXPECT_NEAR(light.direction.y, expected.y, epsilon);
    EXPECT_NEAR(light.direction.z, expected.z, epsilon);
}

TEST_F(DirectionalLightTest, ColorConstructor) {
    Vector3 dir(1.0f, 0.0f, 0.0f);
    LightColor color(1.0f, 0.5f, 0.2f, 2.0f);
    DirectionalLight light(dir, color);
    
    // Direction should be normalized
    EXPECT_NEAR(light.direction.x, 1.0f, epsilon);
    EXPECT_NEAR(light.direction.y, 0.0f, epsilon);
    EXPECT_NEAR(light.direction.z, 0.0f, epsilon);
    
    // Color should match
    EXPECT_NEAR(light.color.r, 1.0f, epsilon);
    EXPECT_NEAR(light.color.g, 0.5f, epsilon);
    EXPECT_NEAR(light.color.b, 0.2f, epsilon);
    EXPECT_NEAR(light.color.i, 2.0f, epsilon);
}

TEST_F(DirectionalLightTest, SetDirection) {
    DirectionalLight light;
    Vector3 dir(1.0f, 1.0f, 1.0f);
    light.setDirection(dir);
    
    // Direction should be normalized
    Vector3 expected = dir.normalized();
    EXPECT_NEAR(light.direction.x, expected.x, epsilon);
    EXPECT_NEAR(light.direction.y, expected.y, epsilon);
    EXPECT_NEAR(light.direction.z, expected.z, epsilon);
}

TEST_F(DirectionalLightTest, ComputeShadowBounds) {
    DirectionalLight light;
    Vector3 center(0.0f, 0.0f, 0.0f);
    float radius = 10.0f;
    Vector3 min, max;
    
    // Light pointing straight down (-Y)
    light.computeShadowBounds(center, radius, min, max);
    
    // Check bounds
    EXPECT_NEAR(min.x, -10.0f, epsilon);  // -radius
    EXPECT_NEAR(min.y, -10.0f, epsilon);  // -radius
    EXPECT_NEAR(min.z, -10.0f, epsilon);  // -radius
    EXPECT_NEAR(max.x, 10.0f, epsilon);   // +radius
    EXPECT_NEAR(max.y, 10.0f, epsilon);   // +radius
    EXPECT_NEAR(max.z, 10.0f, epsilon);   // +radius
    
    // Try with offset center
    Vector3 offsetCenter(5.0f, 5.0f, 5.0f);
    light.computeShadowBounds(offsetCenter, radius, min, max);
    
    EXPECT_NEAR(min.x, -5.0f, epsilon);   // center.x - radius
    EXPECT_NEAR(min.y, -5.0f, epsilon);   // center.y - radius
    EXPECT_NEAR(min.z, -5.0f, epsilon);   // center.z - radius
    EXPECT_NEAR(max.x, 15.0f, epsilon);   // center.x + radius
    EXPECT_NEAR(max.y, 15.0f, epsilon);   // center.y + radius
    EXPECT_NEAR(max.z, 15.0f, epsilon);   // center.z + radius
}
