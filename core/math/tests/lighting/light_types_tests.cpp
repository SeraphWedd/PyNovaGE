#include <gtest/gtest.h>
#include "../../include/lighting/light_types.hpp"

using namespace pynovage::math::lighting;

class LightTypesTest : public ::testing::Test {
protected:
    void SetUp() override {}

    const float epsilon = 1e-6f;
};

// Constructor and Default Value Tests
TEST_F(LightTypesTest, LightColorDefaults) {
    LightColor color;
    EXPECT_FLOAT_EQ(color.r, 1.0f);
    EXPECT_FLOAT_EQ(color.g, 1.0f);
    EXPECT_FLOAT_EQ(color.b, 1.0f);
    EXPECT_FLOAT_EQ(color.i, 1.0f);
}

TEST_F(LightTypesTest, LightColorCustomValues) {
    LightColor color(0.5f, 0.6f, 0.7f, 0.8f);
    EXPECT_FLOAT_EQ(color.r, 0.5f);
    EXPECT_FLOAT_EQ(color.g, 0.6f);
    EXPECT_FLOAT_EQ(color.b, 0.7f);
    EXPECT_FLOAT_EQ(color.i, 0.8f);
}

TEST_F(LightTypesTest, AttenuationParamsDefaults) {
    AttenuationParams params;
    EXPECT_FLOAT_EQ(params.constant, constants::DEFAULT_CONSTANT_ATTENUATION);
    EXPECT_FLOAT_EQ(params.linear, constants::DEFAULT_LINEAR_ATTENUATION);
    EXPECT_FLOAT_EQ(params.quadratic, constants::DEFAULT_QUADRATIC_ATTENUATION);
    EXPECT_FLOAT_EQ(params.range, constants::DEFAULT_POINT_LIGHT_RANGE);
}

TEST_F(LightTypesTest, AttenuationParamsCustomValues) {
    AttenuationParams params(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(params.constant, 1.0f);
    EXPECT_FLOAT_EQ(params.linear, 2.0f);
    EXPECT_FLOAT_EQ(params.quadratic, 3.0f);
    EXPECT_FLOAT_EQ(params.range, 4.0f);
}

TEST_F(LightTypesTest, LightPropertiesDefaults) {
    LightProperties props;
    EXPECT_EQ(props.type, LightType::Point);
    EXPECT_EQ(props.features, LightFeatures::None);
    EXPECT_EQ(props.attenuationModel, AttenuationModel::Smooth);
    
    // Check color defaults
    EXPECT_FLOAT_EQ(props.color.r, 1.0f);
    EXPECT_FLOAT_EQ(props.color.g, 1.0f);
    EXPECT_FLOAT_EQ(props.color.b, 1.0f);
    EXPECT_FLOAT_EQ(props.color.i, 1.0f);
    
    // Check attenuation defaults
    EXPECT_FLOAT_EQ(props.attenuation.constant, constants::DEFAULT_CONSTANT_ATTENUATION);
    EXPECT_FLOAT_EQ(props.attenuation.linear, constants::DEFAULT_LINEAR_ATTENUATION);
    EXPECT_FLOAT_EQ(props.attenuation.quadratic, constants::DEFAULT_QUADRATIC_ATTENUATION);
    EXPECT_FLOAT_EQ(props.attenuation.range, constants::DEFAULT_POINT_LIGHT_RANGE);
}

// SIMD Alignment Tests
TEST_F(LightTypesTest, StructureAlignment) {
    EXPECT_EQ(alignof(LightColor), 16);
    EXPECT_EQ(alignof(AttenuationParams), 16);
    EXPECT_EQ(alignof(LightProperties), 16);
    
    // Verify sizes are multiples of 16 for SIMD
    EXPECT_EQ(sizeof(LightColor) % 16, 0);
    EXPECT_EQ(sizeof(AttenuationParams) % 16, 0);
    EXPECT_EQ(sizeof(LightProperties) % 16, 0);
}

// Enum and Flag Tests
TEST_F(LightTypesTest, LightFeaturesFlags) {
    LightFeatures flags = LightFeatures::None;
    EXPECT_EQ(static_cast<uint32_t>(flags), 0);

    flags = static_cast<LightFeatures>(
        static_cast<uint32_t>(LightFeatures::CastShadows) |
        static_cast<uint32_t>(LightFeatures::UseInverseSquare));
    
    // Test individual flags
    EXPECT_NE(static_cast<uint32_t>(flags) & static_cast<uint32_t>(LightFeatures::CastShadows), 0);
    EXPECT_NE(static_cast<uint32_t>(flags) & static_cast<uint32_t>(LightFeatures::UseInverseSquare), 0);
    EXPECT_EQ(static_cast<uint32_t>(flags) & static_cast<uint32_t>(LightFeatures::VolumetricEnabled), 0);
}

// Range-based Attenuation Tests
TEST_F(LightTypesTest, AttenuationParamsForRange) {
    float testRange = 100.0f;
    AttenuationParams params = AttenuationParams::ForRange(testRange);
    
    // Check computed values
    EXPECT_FLOAT_EQ(params.constant, 1.0f);
    EXPECT_FLOAT_EQ(params.linear, 4.0f / testRange);
    EXPECT_FLOAT_EQ(params.quadratic, 8.0f / (testRange * testRange));
    EXPECT_FLOAT_EQ(params.range, testRange);
    
    // Test with minimum range
    params = AttenuationParams::ForRange(constants::MIN_LIGHT_RANGE);
    EXPECT_GE(params.range, constants::MIN_LIGHT_RANGE);
    
    // Test with maximum range
    params = AttenuationParams::ForRange(constants::MAX_LIGHT_RANGE);
    EXPECT_LE(params.range, constants::MAX_LIGHT_RANGE);
}

// Constants Validation Tests
TEST_F(LightTypesTest, ConstantsValidation) {
    // Range constants
    EXPECT_GT(constants::DEFAULT_POINT_LIGHT_RANGE, 0.0f);
    EXPECT_GT(constants::DEFAULT_SPOT_LIGHT_RANGE, 0.0f);
    EXPECT_GT(constants::MIN_LIGHT_RANGE, 0.0f);
    EXPECT_GT(constants::MAX_LIGHT_RANGE, constants::MIN_LIGHT_RANGE);
    
    // Attenuation constants
    EXPECT_GT(constants::DEFAULT_CONSTANT_ATTENUATION, 0.0f);
    EXPECT_GE(constants::DEFAULT_LINEAR_ATTENUATION, 0.0f);
    EXPECT_GE(constants::DEFAULT_QUADRATIC_ATTENUATION, 0.0f);
    
    // Angle constants
    EXPECT_GT(constants::DEFAULT_SPOT_INNER_ANGLE, 0.0f);
    EXPECT_GT(constants::DEFAULT_SPOT_OUTER_ANGLE, 0.0f);
    EXPECT_GT(constants::DEFAULT_SPOT_OUTER_ANGLE, constants::DEFAULT_SPOT_INNER_ANGLE);
    
    // Threshold constants
    EXPECT_GT(constants::MINIMUM_LIGHT_INTENSITY, 0.0f);
    EXPECT_LT(constants::MINIMUM_LIGHT_INTENSITY, 1.0f);
}

// Edge Case Tests
TEST_F(LightTypesTest, EdgeCases) {
    // Test zero intensity light
    LightColor zeroColor(1.0f, 1.0f, 1.0f, 0.0f);
    EXPECT_FLOAT_EQ(zeroColor.i, 0.0f);
    
    // Test zero range attenuation
    AttenuationParams zeroRange(1.0f, 0.0f, 0.0f, 0.0f);
    EXPECT_FLOAT_EQ(zeroRange.range, 0.0f);
    
    // Test maximum values
    LightColor maxColor(std::numeric_limits<float>::max(),
                       std::numeric_limits<float>::max(),
                       std::numeric_limits<float>::max(),
                       std::numeric_limits<float>::max());
    EXPECT_GT(maxColor.r, 1.0f);
    EXPECT_GT(maxColor.g, 1.0f);
    EXPECT_GT(maxColor.b, 1.0f);
    EXPECT_GT(maxColor.i, 1.0f);
}