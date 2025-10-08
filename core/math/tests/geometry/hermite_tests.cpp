#include "../../include/geometry/hermite.hpp"
#include <gtest/gtest.h>

namespace pynovage {
namespace math {
namespace tests {

class HermiteTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Standard test curve
        p0 = Vector3(0.0f, 0.0f, 0.0f);
        p1 = Vector3(1.0f, 1.0f, 0.0f);
        t0 = Vector3(1.0f, 0.0f, 0.0f);
        t1 = Vector3(1.0f, 0.0f, 0.0f);
    }

    Vector3 p0, p1;  // End points
    Vector3 t0, t1;  // Tangents
};

TEST_F(HermiteTest, Construction) {
    ASSERT_NO_THROW({
        Hermite curve(p0, p1, t0, t1);
    });

    EXPECT_THROW({
        Hermite curve(p0, p1, t0, t1, -1.0f);  // Negative tension
    }, std::invalid_argument);
}

TEST_F(HermiteTest, EndpointInterpolation) {
    Hermite curve(p0, p1, t0, t1);

    // Test endpoint interpolation
    EXPECT_EQ(curve.evaluate(0.0f), p0);
    EXPECT_EQ(curve.evaluate(1.0f), p1);
}

TEST_F(HermiteTest, TangentInfluence) {
    Hermite curve(p0, p1, t0, t1);

    // At t=0, curve should follow start tangent
    Vector3 near_start = curve.evaluate(0.01f);
    Vector3 expected_direction = t0.normalized();
    Vector3 actual_direction = (near_start - p0).normalized();
    
    // Directions should be very close
    EXPECT_NEAR(expected_direction.dot(actual_direction), 1.0f, 0.01f);
}

TEST_F(HermiteTest, TensionEffect) {
    // Compare curves with different tension
    Hermite curve1(p0, p1, t0, t1, 1.0f);
    Hermite curve2(p0, p1, t0, t1, 2.0f);

    // Higher tension should result in stronger tangent influence
    Vector3 point1 = curve1.evaluate(0.25f);
    Vector3 point2 = curve2.evaluate(0.25f);

    // Point2 should deviate more from the straight line between p0 and p1
    Vector3 straight_line = p0 + (p1 - p0) * 0.25f;
    float deviation1 = (point1 - straight_line).length();
    float deviation2 = (point2 - straight_line).length();

    EXPECT_GT(deviation2, deviation1);
}

TEST_F(HermiteTest, BatchEvaluation) {
    Hermite curve(p0, p1, t0, t1);
    std::vector<float> params = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};

    auto points = curve.evaluateMultiple(params);
    ASSERT_EQ(points.size(), params.size());

    // Compare batch results with individual evaluation
    for (size_t i = 0; i < params.size(); ++i) {
        Vector3 expected = curve.evaluate(params[i]);
        EXPECT_EQ(points[i], expected);
    }
}

TEST_F(HermiteTest, Derivative) {
    Hermite curve(p0, p1, t0, t1);
    Hermite derivative = curve.derivative();

    // Test derivative properties
    // 1. At t=0, derivative should equal 3 * initial tangent
    EXPECT_EQ(derivative.evaluate(0.0f), t0 * 3.0f);

    // 2. At t=1, derivative should equal 3 * final tangent
    EXPECT_EQ(derivative.evaluate(1.0f), t1 * 3.0f);
}

TEST_F(HermiteTest, TensionModification) {
    Hermite curve(p0, p1, t0, t1);

    EXPECT_NO_THROW({
        curve.setTension(2.0f);
        EXPECT_FLOAT_EQ(curve.getTension(), 2.0f);
    });

    EXPECT_THROW({
        curve.setTension(-1.0f);
    }, std::invalid_argument);
}

TEST_F(HermiteTest, LargeParameterCount) {
    Hermite curve(p0, p1, t0, t1);
    std::vector<float> large_params(10001, 0.5f);

    EXPECT_THROW({
        curve.evaluateMultiple(large_params);
    }, std::invalid_argument);
}

TEST_F(HermiteTest, GetterMethods) {
    Hermite curve(p0, p1, t0, t1, 1.5f);

    EXPECT_EQ(curve.getStartPoint(), p0);
    EXPECT_EQ(curve.getEndPoint(), p1);
    EXPECT_EQ(curve.getStartTangent(), t0);
    EXPECT_EQ(curve.getEndTangent(), t1);
    EXPECT_FLOAT_EQ(curve.getTension(), 1.5f);
}

} // namespace tests
} // namespace math
} // namespace pynovage