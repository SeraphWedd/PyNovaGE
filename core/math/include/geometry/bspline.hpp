#ifndef PYNOVAGE_MATH_GEOMETRY_BSPLINE_HPP
#define PYNOVAGE_MATH_GEOMETRY_BSPLINE_HPP

#include "../vector3.hpp"
#include <vector>
#include <stdexcept>
#include <algorithm>

namespace pynovage {
namespace math {

/**
 * @brief A class implementing B-spline curves with SIMD optimization
 * 
 * This class provides functionality for creating and evaluating B-spline curves.
 * B-splines are generalizations of Bézier curves that offer local control and 
 * flexibility through control points and a knot vector.
 *
 * Performance Characteristics:
 * - SIMD optimized curve evaluation
 * - Cache-friendly control point storage
 * - Efficient knot vector operations
 * - Constant-time degree elevation
 * - O(log n) knot insertion
 *
 * Usage Guidelines:
 * - Use for smooth curve interpolation
 * - Prefer uniform knot vectors for regular curves
 * - Higher degrees give smoother curves but more computation
 * - Control points define the curve's shape
 * - Knot vector determines parameterization
 *
 * Example:
 * @code
 * // Create a cubic B-spline with 5 control points
 * std::vector<Vector3> points = {
 *     Vector3(0,0,0), Vector3(1,1,0), 
 *     Vector3(2,0,0), Vector3(3,1,0),
 *     Vector3(4,0,0)
 * };
 * BSpline spline(points, 3); // Cubic curve
 *
 * // Evaluate curve at parameter t
 * Vector3 point = spline.evaluate(0.5f);
 * @endcode
 */
class BSpline {
public:
    /**
     * @brief Constructs a B-spline curve
     * @param controlPoints The control points that define the curve's shape
     * @param degree The degree of the B-spline curve (must be >= 1)
     * @param knots Optional knot vector. If not provided, a uniform knot vector is created
     */
    BSpline(const std::vector<Vector3>& controlPoints, int degree, 
            const std::vector<float>& knots = std::vector<float>());

    /**
     * @brief Evaluates the B-spline curve at parameter t
     * @param t The parameter value (typically in [0,1])
     * @return The point on the curve at parameter t
     */
    Vector3 evaluate(float t) const;

    /**
     * @brief Gets the degree of the B-spline curve
     * @return The degree of the curve
     */
    int getDegree() const { return degree_; }

    /**
     * @brief Gets the number of control points
     * @return The number of control points
     */
    size_t getNumControlPoints() const { return controlPoints_.size(); }

    /**
     * @brief Gets the knot vector
     * @return The knot vector
     */
    const std::vector<float>& getKnots() const { return knots_; }

    /**
     * @brief Gets the control points
     * @return The control points
     */
    const std::vector<Vector3>& getControlPoints() const { return controlPoints_; }

    /**
     * @brief Inserts a new knot into the knot vector
     * @param t The parameter value where to insert the knot
     * @return True if insertion was successful
     */
    bool insertKnot(float t);

    /**
     * @brief Elevates the degree of the B-spline
     * @return True if degree elevation was successful
     */
    bool elevateDegree();

    /**
     * @brief Evaluates multiple points along the curve
     * @param parameters Vector of parameter values
     * @return Vector of points on the curve
     */
    std::vector<Vector3> evaluateMultiple(const std::vector<float>& parameters) const;

    /**
     * @brief Computes the derivative of the B-spline curve
     * @return A new B-spline representing the derivative
     */
    BSpline derivative() const;

private:
    /**
     * @brief Finds the knot span containing parameter t
     * @param t The parameter value
     * @return The index of the knot span
     */
    int findSpan(float t) const;

    /**
     * @brief Computes the basis functions for parameter t
     * @param span The knot span index
     * @param t The parameter value
     * @param basisFuncs Output array for basis functions
     */
    void computeBasisFunctions(int span, float t, std::vector<float>& basisFuncs) const;

    /**
     * @brief Creates a uniform knot vector
     */
    void createUniformKnots();

    /**
     * @brief Validates the knot vector
     * @return True if the knot vector is valid
     */
    bool validateKnots() const;

    /**
     * @brief Returns the multiplicity of a knot value
     * @param u The knot value to check
     * @param tolerance Tolerance for floating point comparison
     * @return The number of times u appears in the knot vector
     */
    size_t getMultiplicity(float u, float tolerance = 1e-6f) const;

    /**
     * @brief Returns unique knot values in ascending order
     * @param tolerance Tolerance for floating point comparison
     * @return Vector of unique knot values
     */
    std::vector<float> getUniqueKnots(float tolerance = 1e-6f) const;

    /**
     * @brief Inserts a knot exactly without numerical error
     * 
     * This helper ensures the knot is inserted with exact
     * arithmetic to avoid shape changes from numerical drift.
     * 
     * @param u The parameter value where to insert the knot
     * @param tolerance Tolerance for finding the knot span
     * @return True if insertion was successful
     */
    bool insertKnotExact(float u, float tolerance = 1e-6f);

    /**
     * @brief Converts B-spline segment to Bézier form
     * 
     * Elevates a segment of the B-spline curve to Bézier form by
     * inserting knots until each internal knot has multiplicity p.
     * This preserves the curve shape exactly.
     * 
     * @param startKnot Index where segment starts
     * @param endKnot Index where segment ends
     * @return True if conversion was successful
     */
    bool toBezierForm(size_t startKnot, size_t endKnot);

    /**
     * @brief Elevates degree of a Bézier segment
     * 
     * Given control points of a degree-p Bézier curve,
     * computes control points for degree p+1 that define
     * the same curve shape.
     * 
     * @param points Input control points (size p+1)
     * @return Elevated control points (size p+2)
     */
    std::vector<Vector3> elevateBezierSegment(
        const std::vector<Vector3>& points) const;

    std::vector<Vector3> controlPoints_;  // Control points defining the curve
    std::vector<float> knots_;           // Knot vector
    int degree_;                         // Degree of the curve
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_GEOMETRY_BSPLINE_HPP