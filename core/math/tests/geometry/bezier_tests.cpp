#include "../../include/geometry/bezier.hpp"
#include <gtest/gtest.h>
#include <cmath>
#include <algorithm>

namespace pynovage {
namespace math {
namespace tests {

// Helpers for numerical comparison
bool almostEqual(float a, float b, float epsilon = 1e-5f) {
    return std::abs(a - b) < epsilon;
}

bool almostEqual(const Vector3& a, const Vector3& b, float epsilon = 1e-5f) {
    return almostEqual(a.x, b.x, epsilon) &&
           almostEqual(a.y, b.y, epsilon) &&
           almostEqual(a.z, b.z, epsilon);
}

TEST(BezierTest, Construction) {
    // Single point (not allowed)
    EXPECT_THROW({
        Bezier curve({Vector3(0,0,0)});
    }, std::invalid_argument);
    
    // Valid constructions
    EXPECT_NO_THROW({
        Bezier linear({Vector3(0,0,0), Vector3(1,1,1)});
        Bezier quadratic({Vector3(0,0,0), Vector3(1,1,1), Vector3(2,0,0)});
        Bezier cubic({Vector3(0,0,0), Vector3(1,1,1), Vector3(2,-1,0), Vector3(3,0,0)});
    });
}

TEST(BezierTest, LinearEvaluation) {
    std::vector<Vector3> points = {
        Vector3(0,0,0),
        Vector3(1,1,1)
    };
    Bezier curve(points);
    
    // Test endpoints
    EXPECT_TRUE(almostEqual(curve.evaluate(0.0f), points[0]));
    EXPECT_TRUE(almostEqual(curve.evaluate(1.0f), points[1]));
    
    // Test midpoint
    EXPECT_TRUE(almostEqual(curve.evaluate(0.5f), Vector3(0.5f, 0.5f, 0.5f)));
    
    // Test quarter points
    EXPECT_TRUE(almostEqual(curve.evaluate(0.25f), Vector3(0.25f, 0.25f, 0.25f)));
    EXPECT_TRUE(almostEqual(curve.evaluate(0.75f), Vector3(0.75f, 0.75f, 0.75f)));
}

TEST(BezierTest, QuadraticEvaluation) {
    std::vector<Vector3> points = {
        Vector3(0,0,0),
        Vector3(1,1,0),
        Vector3(2,0,0)
    };
    Bezier curve(points);
    
    // Test endpoints
    EXPECT_TRUE(almostEqual(curve.evaluate(0.0f), points[0]));
    EXPECT_TRUE(almostEqual(curve.evaluate(1.0f), points[2]));
    
    // Test midpoint - should be at (1, 0.5, 0) due to quadratic interpolation
    Vector3 expectedMid(1.0f, 0.5f, 0.0f);
    EXPECT_TRUE(almostEqual(curve.evaluate(0.5f), expectedMid));
}

TEST(BezierTest, CubicEvaluation) {
    std::vector<Vector3> points = {
        Vector3(0,0,0),
        Vector3(1,1,0),
        Vector3(2,-1,0),
        Vector3(3,0,0)
    };
    Bezier curve(points);
    
    // Test endpoints
    EXPECT_TRUE(almostEqual(curve.evaluate(0.0f), points[0]));
    EXPECT_TRUE(almostEqual(curve.evaluate(1.0f), points[3]));
    
    // Test known points (pre-calculated)
    Vector3 expectedQuarter(0.75f, 0.28125f, 0.0f);
    Vector3 expectedMid(1.5f, 0.0f, 0.0f);
    Vector3 expectedThreeQuarter(2.25f, -0.28125f, 0.0f);
    
    EXPECT_TRUE(almostEqual(curve.evaluate(0.25f), expectedQuarter, 1e-4f));
    EXPECT_TRUE(almostEqual(curve.evaluate(0.5f), expectedMid, 1e-4f));
    EXPECT_TRUE(almostEqual(curve.evaluate(0.75f), expectedThreeQuarter, 1e-4f));
}

TEST(BezierTest, BatchEvaluation) {
    std::vector<Vector3> points = {
        Vector3(0,0,0),
        Vector3(1,1,0),
        Vector3(2,-1,0),
        Vector3(3,0,0)
    };
    Bezier curve(points);
    
    std::vector<float> params = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
    auto results = curve.evaluateMultiple(params);
    
    EXPECT_EQ(results.size(), params.size());
    
    // Compare with individual evaluations
    for (size_t i = 0; i < params.size(); ++i) {
        EXPECT_TRUE(almostEqual(results[i], curve.evaluate(params[i])));
    }
}

TEST(BezierTest, Derivative) {
    // Linear curve - derivative should be constant
    std::vector<Vector3> points1 = {
        Vector3(0,0,0),
        Vector3(1,1,1)
    };
    Bezier linear(points1);
    auto linearDeriv = linear.derivative();
    EXPECT_TRUE(almostEqual(linearDeriv.evaluate(0.0f), Vector3(1,1,1)));
    EXPECT_TRUE(almostEqual(linearDeriv.evaluate(0.5f), Vector3(1,1,1)));
    EXPECT_TRUE(almostEqual(linearDeriv.evaluate(1.0f), Vector3(1,1,1)));
    
    // Quadratic curve - derivative should be linear
    std::vector<Vector3> points2 = {
        Vector3(0,0,0),
        Vector3(1,1,0),
        Vector3(2,0,0)
    };
    Bezier quadratic(points2);
    auto quadraticDeriv = quadratic.derivative();
    EXPECT_EQ(quadraticDeriv.getDegree(), 1);
    
    // Test derivative values at known points
    Vector3 expectedStart(2,2,0);
    Vector3 expectedEnd(2,-2,0);
    EXPECT_TRUE(almostEqual(quadraticDeriv.evaluate(0.0f), expectedStart));
    EXPECT_TRUE(almostEqual(quadraticDeriv.evaluate(1.0f), expectedEnd));
}

TEST(BezierTest, DegreeElevation) {
    // Linear curve
    std::vector<Vector3> points = {
        Vector3(0,0,0),
        Vector3(1,1,1)
    };
    Bezier curve(points);
    
    // Elevate degree
    EXPECT_TRUE(curve.elevateDegree());
    EXPECT_EQ(curve.getDegree(), 2);
    
    // Check that elevated curve matches original at sample points
    std::vector<float> testPoints = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
    for (float t : testPoints) {
        Vector3 expected(t,t,t);  // Linear interpolation
        EXPECT_TRUE(almostEqual(curve.evaluate(t), expected));
    }
}

TEST(BezierTest, DegreeReduction) {
    // Create a quadratic curve that's actually linear
    std::vector<Vector3> points = {
        Vector3(0,0,0),
        Vector3(0.5f,0.5f,0.5f),
        Vector3(1,1,1)
    };
    Bezier curve(points);
    
    // Try to reduce degree
    EXPECT_TRUE(curve.reduceDegree(1e-4f));
    EXPECT_EQ(curve.getDegree(), 1);
    
    // Check that reduced curve matches original at sample points
    std::vector<float> testPoints = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
    for (float t : testPoints) {
        Vector3 expected(t,t,t);
        EXPECT_TRUE(almostEqual(curve.evaluate(t), expected));
    }
}

TEST(BezierTest, Split) {
    std::vector<Vector3> points = {
        Vector3(0,0,0),
        Vector3(1,1,0),
        Vector3(2,-1,0),
        Vector3(3,0,0)
    };
    Bezier curve(points);
    
    // Split at t=0.5
    auto [left, right] = curve.split(0.5f);
    
    // Check endpoints match
    EXPECT_TRUE(almostEqual(left.evaluate(0.0f), curve.evaluate(0.0f)));
    EXPECT_TRUE(almostEqual(left.evaluate(1.0f), curve.evaluate(0.5f)));
    EXPECT_TRUE(almostEqual(right.evaluate(0.0f), curve.evaluate(0.5f)));
    EXPECT_TRUE(almostEqual(right.evaluate(1.0f), curve.evaluate(1.0f)));
    
    // Check intermediate points
    EXPECT_TRUE(almostEqual(left.evaluate(0.5f), curve.evaluate(0.25f)));
    EXPECT_TRUE(almostEqual(right.evaluate(0.5f), curve.evaluate(0.75f)));
}

TEST(BezierTest, NumericalStability) {
    // Test with very small and very large control points
    std::vector<Vector3> points = {
        Vector3(0,0,0),
        Vector3(1e-6f,1e-6f,1e-6f),
        Vector3(1e6f,1e6f,1e6f),
        Vector3(1e-6f,1e-6f,1e-6f)
    };
    Bezier curve(points);
    
    // Evaluate at multiple points and check results are reasonable
    for (float t = 0.0f; t <= 1.0f; t += 0.1f) {
        Vector3 result = curve.evaluate(t);
        EXPECT_FALSE(std::isnan(result.x));
        EXPECT_FALSE(std::isnan(result.y));
        EXPECT_FALSE(std::isnan(result.z));
        EXPECT_FALSE(std::isinf(result.x));
        EXPECT_FALSE(std::isinf(result.y));
        EXPECT_FALSE(std::isinf(result.z));
    }
}

TEST(BezierTest, EdgeCases) {
    // Test with coincident control points
    std::vector<Vector3> points = {
        Vector3(1,1,1),
        Vector3(1,1,1),
        Vector3(1,1,1)
    };
    Bezier curve(points);
    
    // All evaluations should return the same point
    for (float t = 0.0f; t <= 1.0f; t += 0.1f) {
        EXPECT_TRUE(almostEqual(curve.evaluate(t), Vector3(1,1,1)));
    }
    
    // Test derivative of constant curve
    auto deriv = curve.derivative();
    EXPECT_TRUE(almostEqual(deriv.evaluate(0.5f), Vector3(0,0,0)));
    
    // Test with parameters outside [0,1]
    EXPECT_TRUE(almostEqual(curve.evaluate(-1.0f), points.front()));
    EXPECT_TRUE(almostEqual(curve.evaluate(2.0f), points.back()));
}

} // namespace tests
} // namespace math
} // namespace pynovage