#include "../../include/geometry/catmull_rom.hpp"
#include <gtest/gtest.h>

namespace pynovage {
namespace math {
namespace tests {

class CatmullRomTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Curve with non-uniform segment lengths to test parameterization effects
        points = {
            Vector3(0.0f, 0.0f, 0.0f),   // P0: origin
            Vector3(1.0f, 0.0f, 0.0f),   // P1: unit step right
            Vector3(1.0f, 2.0f, 0.0f),   // P2: 2 units up (longer segment)
            Vector3(3.0f, 2.0f, 0.0f)    // P3: 2 units right (longer segment)
        };
        // This creates segments with lengths:
        // P0->P1 = 1.0 (unit step)
        // P1->P2 = 2.0 (double length)
        // P2->P3 = 2.0 (double length)
        // The non-uniform lengths mean different parameterizations will
        // produce different curves:
        // - Uniform: equal time between points regardless of distance
        // - Centripetal: time proportional to sqrt(distance)
        // - Chordal: time directly proportional to distance
    }

    std::vector<Vector3> points;
};

TEST_F(CatmullRomTest, Construction) {
    ASSERT_NO_THROW({
        CatmullRom spline(points);
    });

    // Test with minimum points
    std::vector<Vector3> min_points = {Vector3(0,0,0), Vector3(1,1,1)};
    ASSERT_NO_THROW({
        CatmullRom spline(min_points);
    });

    // Test with insufficient points
    std::vector<Vector3> single_point = {Vector3(0,0,0)};
    EXPECT_THROW({
        CatmullRom spline(single_point);
    }, std::invalid_argument);
}

TEST_F(CatmullRomTest, PointInterpolation) {
    CatmullRom spline(points);

    // Test key point interpolation
    // For Catmull-Rom, the spline interpolates P1 to P2 (points[1] to points[2])
    EXPECT_EQ(spline.evaluate(0.0f), points[1]);
    EXPECT_EQ(spline.evaluate(1.0f), points[2]);
}

TEST_F(CatmullRomTest, Parameterization) {
    // Test different parameterization types
    CatmullRom uniform(points, CatmullRom::Parameterization::Uniform);
    CatmullRom centripetal(points, CatmullRom::Parameterization::Centripetal);
    CatmullRom chordal(points, CatmullRom::Parameterization::Chordal);

    // Points at t=0.5 should differ between parameterizations
    Vector3 p_uniform = uniform.evaluate(0.5f);
    Vector3 p_centripetal = centripetal.evaluate(0.5f);
    Vector3 p_chordal = chordal.evaluate(0.5f);

    // Points should be different (not testing exact values)
    EXPECT_NE(p_uniform, p_centripetal);
    EXPECT_NE(p_uniform, p_chordal);
    EXPECT_NE(p_centripetal, p_chordal);
}

TEST_F(CatmullRomTest, TensionEffect) {
    // Compare curves with different tension
    CatmullRom curve1(points, CatmullRom::Parameterization::Uniform, 0.5f);
    CatmullRom curve2(points, CatmullRom::Parameterization::Uniform, 2.0f);

    // Higher tension should result in stronger tangent influence
    Vector3 point1 = curve1.evaluate(0.5f);
    Vector3 point2 = curve2.evaluate(0.5f);

    // Point2 should deviate more from the straight line between P1 and P2
    Vector3 straight_line = points[1] + (points[2] - points[1]) * 0.5f;
    float deviation1 = (point1 - straight_line).length();
    float deviation2 = (point2 - straight_line).length();

    EXPECT_GT(deviation2, deviation1);
}

TEST_F(CatmullRomTest, BatchEvaluation) {
    CatmullRom spline(points);
    std::vector<float> params = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};

    auto points_batch = spline.evaluateMultiple(params);
    ASSERT_EQ(points_batch.size(), params.size());

    // Compare batch results with individual evaluation
    for (size_t i = 0; i < params.size(); ++i) {
        Vector3 expected = spline.evaluate(params[i]);
        EXPECT_EQ(points_batch[i], expected);
    }
}

TEST_F(CatmullRomTest, Derivative) {
    CatmullRom spline(points);

    // Test derivative at key points
    Vector3 start_deriv = spline.derivative(0.0f);
    Vector3 end_deriv = spline.derivative(1.0f);

    // Derivatives should point in reasonable directions
    EXPECT_GT(start_deriv.dot(points[2] - points[1]), 0.0f);  // Points toward next point
    EXPECT_GT(end_deriv.dot(points[2] - points[1]), 0.0f);    // Points in curve direction
}

TEST_F(CatmullRomTest, PointManipulation) {
    CatmullRom spline(points);
    size_t original_size = points.size();

    // Test point addition
    Vector3 new_point(3.0f, 1.0f, 0.0f);
    EXPECT_NO_THROW({
        spline.addPoint(new_point);
    });
    EXPECT_EQ(spline.getControlPoints().size(), original_size + 1);

    // Test point insertion
    Vector3 insert_point(1.5f, 0.5f, 0.0f);
    EXPECT_NO_THROW({
        spline.insertPoint(insert_point, 2);
    });
    EXPECT_EQ(spline.getControlPoints().size(), original_size + 2);

    // Test point removal
    EXPECT_NO_THROW({
        spline.removePoint(2);
    });
    EXPECT_EQ(spline.getControlPoints().size(), original_size + 1);

    // Test invalid operations
    EXPECT_THROW({
        spline.insertPoint(Vector3(), spline.getControlPoints().size() + 1);
    }, std::out_of_range);

    EXPECT_THROW({
        spline.removePoint(spline.getControlPoints().size());
    }, std::out_of_range);
}

TEST_F(CatmullRomTest, ParameterizationChange) {
    CatmullRom spline(points);
    Vector3 point_before = spline.evaluate(0.5f);

    // Change parameterization
    spline.setParameterization(CatmullRom::Parameterization::Chordal);
    Vector3 point_after = spline.evaluate(0.5f);

    // Point should be different with new parameterization
    EXPECT_NE(point_before, point_after);
}

TEST_F(CatmullRomTest, TensionModification) {
    CatmullRom spline(points);

    EXPECT_NO_THROW({
        spline.setTension(2.0f);
        EXPECT_FLOAT_EQ(spline.getTension(), 2.0f);
    });

    EXPECT_THROW({
        spline.setTension(-1.0f);
    }, std::invalid_argument);
}

TEST_F(CatmullRomTest, LargeParameterCount) {
    CatmullRom spline(points);
    std::vector<float> large_params(10001, 0.5f);

    EXPECT_THROW({
        spline.evaluateMultiple(large_params);
    }, std::invalid_argument);
}

} // namespace tests
} // namespace math
} // namespace pynovage