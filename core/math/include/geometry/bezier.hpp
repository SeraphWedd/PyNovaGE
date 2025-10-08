#ifndef PYNOVAGE_MATH_GEOMETRY_BEZIER_HPP
#define PYNOVAGE_MATH_GEOMETRY_BEZIER_HPP

#include "../vector3.hpp"
#include "../simd_utils.hpp"
#include <vector>
#include <algorithm>
#include <cmath>

namespace pynovage {
namespace math {

/**
 * @brief A class implementing Bézier curves with SIMD optimization
 * 
 * This class provides functionality for creating and evaluating Bézier curves.
 * Bézier curves are parametric curves that use Bernstein polynomials as a basis.
 * They are commonly used for smooth interpolation between points.
 *
 * Performance Characteristics:
 * - SIMD-optimized curve evaluation
 * - Cache-friendly control point storage
 * - Efficient basis computation
 * - Fast degree elevation/reduction
 * - Optimized subdivision
 *
 * Usage Guidelines:
 * - Use for smooth interpolation between points
 * - Lower degrees give more predictable results
 * - Higher degrees allow more complex shapes
 * - Control points define the curve shape
 * - Curve always passes through first and last points
 *
 * Example:
 * @code
 * // Create a cubic Bézier curve with 4 control points
 * std::vector<Vector3> points = {
 *     Vector3(0,0,0), Vector3(1,1,0), 
 *     Vector3(2,-1,0), Vector3(3,0,0)
 * };
 * Bezier curve(points);
 *
 * // Evaluate curve at parameter t
 * Vector3 point = curve.evaluate(0.5f);
 * @endcode
 */
class Bezier {
public:
    /**
     * @brief Constructs a Bézier curve
     * @param controlPoints The control points that define the curve's shape
     * @throw std::invalid_argument if fewer than 2 control points are provided
     */
    explicit Bezier(const std::vector<Vector3>& controlPoints);

    /**
     * @brief Evaluates the Bézier curve at parameter t
     * @param t The parameter value in [0,1]
     * @return The point on the curve at parameter t
     */
    Vector3 evaluate(float t) const;

    /**
     * @brief Evaluates multiple points along the curve efficiently
     * @param parameters Vector of parameter values in [0,1]
     * @return Vector of points on the curve
     */
    std::vector<Vector3> evaluateMultiple(const std::vector<float>& parameters) const;

    /**
     * @brief Computes the derivative of the Bézier curve
     * @return A new Bézier curve representing the derivative
     */
    Bezier derivative() const;

    /**
     * @brief Elevates the degree of the curve while maintaining its shape
     * @return True if degree elevation was successful
     */
    bool elevateDegree();

    /**
     * @brief Reduces the degree of the curve while approximating its shape
     * @param maxError Maximum allowed error for the approximation
     * @return True if degree reduction was successful within error bounds
     */
    bool reduceDegree(float maxError = 1e-4f);

    /**
     * @brief Splits the curve at parameter t
     * @param t The parameter value in [0,1]
     * @return Pair of Bézier curves representing left and right parts
     */
    std::pair<Bezier, Bezier> split(float t) const;

    /**
     * @brief Gets the degree of the Bézier curve
     * @return The degree (number of control points - 1)
     */
    int getDegree() const { return static_cast<int>(controlPoints_.size()) - 1; }

    /**
     * @brief Gets the control points
     * @return Vector of control points
     */
    const std::vector<Vector3>& getControlPoints() const { return controlPoints_; }

private:
    /**
     * @brief Computes Bernstein basis polynomials
     * @param t Parameter value
     * @return Vector of basis values
     */
    std::vector<float> computeBasis(float t) const;

    /**
     * @brief SIMD-optimized basis computation
     * @param t Parameter value
     * @param basis Output array for basis values
     */
    void computeBasisSIMD(float t, float* basis) const;

    /**
     * @brief De Casteljau's algorithm for curve evaluation
     * @param t Parameter value
     * @return Point on curve at parameter t
     */
    Vector3 evaluateDeCasteljau(float t) const;

    /**
     * @brief SIMD-optimized curve evaluation
     * @param t Parameter value
     * @return Point on curve at parameter t
     */
    Vector3 evaluateSIMD(float t) const;

    /**
     * @brief Computes binomial coefficients for the current degree
     * @return Vector of binomial coefficients
     */
    std::vector<int> computeBinomialCoefficients() const;

    /**
     * @brief Validates input control points
     * @param points Vector of control points to validate
     * @throw std::invalid_argument if validation fails
     */
    static void validateControlPoints(const std::vector<Vector3>& points);

    std::vector<Vector3> controlPoints_;  // Control points defining the curve
    std::vector<int> binomialCoeffs_;    // Cached binomial coefficients
    bool useSimd_;                       // Whether to use SIMD operations
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_GEOMETRY_BEZIER_HPP