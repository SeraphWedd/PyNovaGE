#include <gtest/gtest.h>
#include "../../include/lighting/attenuation.hpp"
#include "../../include/vector3.hpp"

using namespace pynovage::math::lighting;
using pynovage::math::Vector3;

class AttenuationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup default attenuation parameters for tests
        params = AttenuationParams{};
        params.constant = 1.0f;
        params.linear = 0.0f;
        params.quadratic = 0.0f;
        params.range = 100.0f;
        
        // Default test positions
        lightPos = Vector3(0.0f, 0.0f, 0.0f);
        pointA = Vector3(1.0f, 0.0f, 0.0f);    // 1 unit away
        pointB = Vector3(2.0f, 0.0f, 0.0f);    // 2 units away
        pointC = Vector3(0.0f, 5.0f, 0.0f);    // 5 units away
        pointFar = Vector3(0.0f, 0.0f, 150.0f); // Beyond range
    }

    const float epsilon = 1e-6f;
    AttenuationParams params;
    Vector3 lightPos, pointA, pointB, pointC, pointFar;
};
// Basic Attenuation Tests
TEST_F(AttenuationTest, LinearAttenuation) {
    params.constant = 1.0f;
    params.linear = 1.0f;
    params.quadratic = 0.0f;
    
    float attenA = calculateAttenuation(params, lightPos, pointA, AttenuationModel::Linear);
    float attenB = calculateAttenuation(params, lightPos, pointB, AttenuationModel::Linear);
    
    // At distance 1, attenuation should be 1/(1 + 1) = 0.5
    EXPECT_NEAR(attenA, 0.5f, epsilon);
    // At distance 2, attenuation should be 1/(1 + 2) = 0.333...
    EXPECT_NEAR(attenB, 1.0f / 3.0f, epsilon);
}

TEST_F(AttenuationTest, QuadraticAttenuation) {
    params.constant = 1.0f;
    params.linear = 0.0f;
    params.quadratic = 1.0f;
    
    float attenA = calculateAttenuation(params, lightPos, pointA, AttenuationModel::InverseSquare);
    float attenB = calculateAttenuation(params, lightPos, pointB, AttenuationModel::InverseSquare);
    
    // At distance 1, attenuation should be 1/(1 + 1) = 0.5
    EXPECT_NEAR(attenA, 0.5f, epsilon);
    // At distance 2, attenuation should be 1/(1 + 4) = 0.2
    EXPECT_NEAR(attenB, 0.2f, epsilon);
}

TEST_F(AttenuationTest, SmoothAttenuation) {
    params.constant = 1.0f;
    params.linear = 0.5f;
    params.quadratic = 0.25f;
    
    float attenA = calculateAttenuation(params, lightPos, pointA, AttenuationModel::Smooth);
    float attenB = calculateAttenuation(params, lightPos, pointB, AttenuationModel::Smooth);
    
    // At distance 1: 1/(1 + 0.5 + 0.25) = 0.571428...
    EXPECT_NEAR(attenA, 1.0f / 1.75f, epsilon);
    // At distance 2: 1/(1 + 1 + 1) = 0.333...
    EXPECT_NEAR(attenB, 1.0f / 3.0f, epsilon);
}

// Range Tests
TEST_F(AttenuationTest, RangeBasedAttenuation) {
    params = AttenuationParams::ForRange(5.0f);
    
    // Test points at different distances
    float attenA = calculateAttenuation(params, lightPos, pointA, AttenuationModel::Smooth);
    float attenC = calculateAttenuation(params, lightPos, pointC, AttenuationModel::Smooth);
    float attenFar = calculateAttenuation(params, lightPos, pointFar, AttenuationModel::Smooth);
    
    // Verify attenuation decreases with distance
    EXPECT_GT(attenA, attenC);
    EXPECT_GT(attenC, attenFar);
    
// At range distance, attenuation should be close to the designed curve value
    float attenAtRange = calculateAttenuation(params, lightPos, Vector3(0.0f, 5.0f, 0.0f), AttenuationModel::Smooth);
    // With ForRange(5): constant=1, linear=4/5=0.8, quadratic=8/25=0.32
    // distance=5 -> 1/(1 + 0.8*5 + 0.32*25) = 1/(1 + 4 + 8) = 1/13 ≈ 0.076923
    EXPECT_NEAR(attenAtRange, 1.0f/13.0f, 1e-5f);
}

// Model Comparison Tests
TEST_F(AttenuationTest, AttenuationModelComparison) {
    Vector3 testPoint(3.0f, 4.0f, 0.0f); // 5 units away

    // Set parameters so models differ
    params.constant = 1.0f;
    params.linear = 0.1f;       // gentle linear falloff
    params.quadratic = 0.5f;   // stronger quadratic falloff

    float linearAtten = calculateAttenuation(params, lightPos, testPoint, AttenuationModel::Linear);
    float inverseSquareAtten = calculateAttenuation(params, lightPos, testPoint, AttenuationModel::InverseSquare);
    float smoothAtten = calculateAttenuation(params, lightPos, testPoint, AttenuationModel::Smooth);
    float noAtten = calculateAttenuation(params, lightPos, testPoint, AttenuationModel::None);
    
    // No attenuation should always return 1
    EXPECT_FLOAT_EQ(noAtten, 1.0f);
    
    // Linear should attenuate less aggressively than inverse square at this distance
    EXPECT_GT(linearAtten, inverseSquareAtten);
    
    // Smooth attenuation adds a linear term on top of inverse-square, so it should be less than or equal to inverse-square
    EXPECT_LE(smoothAtten, inverseSquareAtten);
    // And it should attenuate more than linear (i.e., be <= linear)
    EXPECT_LE(smoothAtten, linearAtten);
}

// Edge Cases
TEST_F(AttenuationTest, EdgeCases) {
    // Zero distance
    Vector3 zeroPoint = lightPos;
    float attenZero = calculateAttenuation(params, lightPos, zeroPoint, AttenuationModel::Smooth);
    EXPECT_FLOAT_EQ(attenZero, 1.0f);
    
// Very far point (beyond range)
    float attenVeryFar = calculateAttenuation(params, lightPos, pointFar, AttenuationModel::Smooth);
    EXPECT_FLOAT_EQ(attenVeryFar, 0.0f);
    
    // Zero parameters
    AttenuationParams zeroParams(0.0f, 0.0f, 0.0f, 1.0f);
    float attenZeroParams = calculateAttenuation(zeroParams, lightPos, pointA, AttenuationModel::Smooth);
    EXPECT_GT(attenZeroParams, 0.0f);
}

// SIMD Batch Processing Tests
TEST_F(AttenuationTest, BatchProcessing) {
    const size_t numPoints = 4; // SIMD width
    Vector3 points[numPoints] = {
        Vector3(1.0f, 0.0f, 0.0f),
        Vector3(2.0f, 0.0f, 0.0f),
        Vector3(3.0f, 0.0f, 0.0f),
        Vector3(4.0f, 0.0f, 0.0f)
    };
    
    float results[numPoints];
    calculateAttenuationBatch(params, lightPos, points, numPoints, 
                            AttenuationModel::Smooth, results);
    
    // Verify batch results match individual calculations
    for (size_t i = 0; i < numPoints; ++i) {
        float expected = calculateAttenuation(params, lightPos, points[i], 
                                            AttenuationModel::Smooth);
        EXPECT_NEAR(results[i], expected, epsilon);
    }
}

// Performance Consistency Test
TEST_F(AttenuationTest, DistanceConsistency) {
    // Use parameters that decrease with distance
    params.constant = 1.0f;
    params.linear = 0.3f;
    params.quadratic = 0.05f;
    // Test that attenuation is consistent regardless of direction
    Vector3 pointsX(5.0f, 0.0f, 0.0f);
    Vector3 pointsY(0.0f, 5.0f, 0.0f);
    Vector3 pointsZ(0.0f, 0.0f, 5.0f);
    Vector3 pointsDiagonal(3.0f, 3.0f, 3.0f); // ~5.2 units
    
    float attenX = calculateAttenuation(params, lightPos, pointsX, AttenuationModel::Smooth);
    float attenY = calculateAttenuation(params, lightPos, pointsY, AttenuationModel::Smooth);
    float attenZ = calculateAttenuation(params, lightPos, pointsZ, AttenuationModel::Smooth);
    
    // Attenuation should be equal for equal distances regardless of direction
    EXPECT_NEAR(attenX, attenY, epsilon);
    EXPECT_NEAR(attenY, attenZ, epsilon);
    
    // Diagonal should have slightly more attenuation due to greater distance
    float attenDiag = calculateAttenuation(params, lightPos, pointsDiagonal, AttenuationModel::Smooth);
    EXPECT_LT(attenDiag, attenX);
}

