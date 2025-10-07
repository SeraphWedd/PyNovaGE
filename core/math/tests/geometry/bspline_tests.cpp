#include "../../include/geometry/bspline.hpp"
#include <gtest/gtest.h>
#include <cmath>
#include <memory>

namespace pynovage {
namespace math {
namespace tests {

class BSplineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple quadratic B-spline with 4 control points
        controlPoints = {
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(1.0f, 1.0f, 0.0f),
            Vector3(2.0f, 0.0f, 0.0f),
            Vector3(3.0f, 1.0f, 0.0f)
        };
        degree = 2;  // Quadratic
        spline = std::make_unique<BSpline>(controlPoints, degree);
    }

    std::vector<Vector3> controlPoints;
    int degree;
    std::unique_ptr<BSpline> spline;
};

TEST_F(BSplineTest, Constructor) {
    ASSERT_NO_THROW(BSpline(controlPoints, degree));
    ASSERT_THROW(BSpline({}, degree), std::invalid_argument);
    ASSERT_THROW(BSpline(controlPoints, 0), std::invalid_argument);
    ASSERT_THROW(BSpline({Vector3()}, 2), std::invalid_argument);
}

TEST_F(BSplineTest, Getters) {
    EXPECT_EQ(spline->getDegree(), degree);
    EXPECT_EQ(spline->getNumControlPoints(), controlPoints.size());
    EXPECT_EQ(spline->getControlPoints().size(), controlPoints.size());
    
    // Verify knot vector size: n + p + 2 (where n is number of control points - 1)
    size_t expectedKnots = (controlPoints.size() - 1) + degree + 2;
    EXPECT_EQ(spline->getKnots().size(), expectedKnots);
}

TEST_F(BSplineTest, KnotVector) {
    const auto& knots = spline->getKnots();
    
    // Verify knot vector is non-decreasing
    for (size_t i = 1; i < knots.size(); ++i) {
        EXPECT_GE(knots[i], knots[i-1]);
    }

    // Verify first and last knots
    EXPECT_FLOAT_EQ(knots.front(), 0.0f);
    EXPECT_FLOAT_EQ(knots.back(), 1.0f);
}

TEST_F(BSplineTest, Evaluation) {
    // Test curve evaluation at specific points
    Vector3 start = spline->evaluate(0.0f);
    Vector3 end = spline->evaluate(1.0f);
    Vector3 mid = spline->evaluate(0.5f);

    // Start should be near first control point
    EXPECT_NEAR(start.x, controlPoints.front().x, 1e-5f);
    EXPECT_NEAR(start.y, controlPoints.front().y, 1e-5f);
    EXPECT_NEAR(start.z, controlPoints.front().z, 1e-5f);

    // End should be near last control point
    EXPECT_NEAR(end.x, controlPoints.back().x, 1e-5f);
    EXPECT_NEAR(end.y, controlPoints.back().y, 1e-5f);
    EXPECT_NEAR(end.z, controlPoints.back().z, 1e-5f);

    // Mid point should be between control points
    EXPECT_GT(mid.x, start.x);
    EXPECT_LT(mid.x, end.x);
}

TEST_F(BSplineTest, MultipleEvaluation) {
    std::vector<float> params = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
    auto points = spline->evaluateMultiple(params);

    EXPECT_EQ(points.size(), params.size());

    // Compare with individual evaluations
    for (size_t i = 0; i < params.size(); ++i) {
        Vector3 expected = spline->evaluate(params[i]);
        EXPECT_NEAR(points[i].x, expected.x, 1e-5f);
        EXPECT_NEAR(points[i].y, expected.y, 1e-5f);
        EXPECT_NEAR(points[i].z, expected.z, 1e-5f);
    }
}

TEST_F(BSplineTest, KnotInsertion) {
    size_t originalPoints = spline->getNumControlPoints();
    size_t originalKnots = spline->getKnots().size();

    EXPECT_TRUE(spline->insertKnot(0.5f));

    // Verify number of control points and knots increased
    EXPECT_EQ(spline->getNumControlPoints(), originalPoints + 1);
    EXPECT_EQ(spline->getKnots().size(), originalKnots + 1);

    // Verify curve shape preserved
    std::vector<float> params = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
    auto origPoints = spline->evaluateMultiple(params);
    EXPECT_TRUE(spline->insertKnot(0.75f));
    auto newPoints = spline->evaluateMultiple(params);

    // Points should be very close (within numerical precision)
    for (size_t i = 0; i < params.size(); ++i) {
        EXPECT_NEAR(origPoints[i].x, newPoints[i].x, 1e-4f);
        EXPECT_NEAR(origPoints[i].y, newPoints[i].y, 1e-4f);
        EXPECT_NEAR(origPoints[i].z, newPoints[i].z, 1e-4f);
    }
}

TEST_F(BSplineTest, DegreeElevation) {
    int originalDegree = spline->getDegree();
    size_t originalPoints = spline->getNumControlPoints();

    EXPECT_TRUE(spline->elevateDegree());

    // Verify degree increased
    EXPECT_EQ(spline->getDegree(), originalDegree + 1);
    EXPECT_EQ(spline->getNumControlPoints(), originalPoints + 1);

    // Verify curve shape preserved
    std::vector<float> params = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
    auto origElevPoints = spline->evaluateMultiple(params);
    EXPECT_TRUE(spline->elevateDegree());
    auto newPoints = spline->evaluateMultiple(params);

    // Points should be very close (within numerical precision)
    for (size_t i = 0; i < params.size(); ++i) {
        EXPECT_NEAR(origElevPoints[i].x, newPoints[i].x, 1e-4f);
        EXPECT_NEAR(origElevPoints[i].y, newPoints[i].y, 1e-4f);
        EXPECT_NEAR(origElevPoints[i].z, newPoints[i].z, 1e-4f);
    }
}

TEST_F(BSplineTest, Derivative) {
    auto derivative = spline->derivative();

    // Verify derivative properties
    EXPECT_EQ(derivative.getDegree(), spline->getDegree() - 1);
    EXPECT_EQ(derivative.getNumControlPoints(), spline->getNumControlPoints() - 1);

    // Test derivative at specific points
    float h = 1e-4f;
    float t = 0.5f;
    Vector3 p1 = spline->evaluate(t);
    Vector3 p2 = spline->evaluate(t + h);
    Vector3 numericalDeriv = (p2 - p1) * (1.0f / h);
    Vector3 analyticalDeriv = derivative.evaluate(t);

    // Compare numerical and analytical derivatives
    EXPECT_NEAR(numericalDeriv.x, analyticalDeriv.x, 1e-3f);
    EXPECT_NEAR(numericalDeriv.y, analyticalDeriv.y, 1e-3f);
    EXPECT_NEAR(numericalDeriv.z, analyticalDeriv.z, 1e-3f);
}

} // namespace tests
} // namespace math
} // namespace pynovage