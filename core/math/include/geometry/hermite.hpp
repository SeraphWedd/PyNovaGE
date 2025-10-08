#ifndef PYNOVAGE_MATH_GEOMETRY_HERMITE_HPP
#define PYNOVAGE_MATH_GEOMETRY_HERMITE_HPP

#include "../vector3.hpp"
#include "../simd_utils.hpp"
#include <vector>
#include <algorithm>
#include <cmath>

namespace pynovage {
namespace math {

/**
 * @brief A class implementing Hermite curves with SIMD optimization
 * 
 * This class provides functionality for creating and evaluating Hermite curves.
 * Hermite curves are defined by two endpoints and their tangent vectors, making
 * them particularly useful for animation and path control where direct control
 * over the curve's direction is desired.
 *
 * Performance Characteristics:
 * - SIMD-optimized curve evaluation
 * - Cache-friendly point and tangent storage
 * - Efficient basis computation
 * - Fast tension parameter adjustment
 * - Optimized batch processing
 *
 * Memory Usage:
 * - Fixed size storage (2 points + 2 tangents)
 * - Stack allocation for small operations
 * - Batch operations use aligned heap storage
 *
 * Usage Guidelines:
 * - Use for animation where tangent control is important
 * - Ideal for camera path smoothing
 * - Good for velocity-controlled movement
 * - Efficient for motion with known start/end velocities
 *
 * Example:
 * @code
 * // Create a Hermite curve with points and tangents
 * Vector3 p0(0,0,0), p1(1,1,0);     // Start and end points
 * Vector3 t0(1,0,0), t1(1,0,0);     // Start and end tangents
 * Hermite curve(p0, p1, t0, t1);
 *
 * // Evaluate curve at parameter t
 * Vector3 point = curve.evaluate(0.5f);
 * @endcode
 */
class Hermite {
public:
    /**
     * @brief Constructs a Hermite curve from points and tangents
     * @param p0 Start point
     * @param p1 End point
     * @param t0 Start tangent (velocity)
     * @param t1 End tangent (velocity)
     * @param tension Tension parameter (default = 1.0)
     */
    Hermite(const Vector3& p0, const Vector3& p1,
           const Vector3& t0, const Vector3& t1,
           float tension = 1.0f);

    /**
     * @brief Evaluates the Hermite curve at parameter t
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
     * @brief Computes the derivative of the Hermite curve
     * @return A new Hermite curve representing the derivative
     */
    Hermite derivative() const;

    /**
     * @brief Sets the tension parameter for the curve
     * @param tension New tension value (affects curve shape)
     */
    void setTension(float tension);

    /**
     * @brief Gets the current tension parameter
     * @return Current tension value
     */
    float getTension() const { return tension_; }

    /**
     * @brief Gets the start point
     * @return The start point of the curve
     */
    const Vector3& getStartPoint() const { return p0_; }

    /**
     * @brief Gets the end point
     * @return The end point of the curve
     */
    const Vector3& getEndPoint() const { return p1_; }

    /**
     * @brief Gets the start tangent
     * @return The start tangent vector
     */
    const Vector3& getStartTangent() const { return t0_; }

    /**
     * @brief Gets the end tangent
     * @return The end tangent vector
     */
    const Vector3& getEndTangent() const { return t1_; }

private:
    /**
     * @brief Computes Hermite basis functions
     * @param t Parameter value
     * @return Array of 4 basis values
     */
    void computeBasis(float t, float* basis) const;

    /**
     * @brief SIMD-optimized basis computation
     * @param t Parameter value
     * @param basis Output array for basis values (size 4)
     */
    void computeBasisSIMD(float t, float* basis) const;

    /**
     * @brief Validates the tension parameter
     * @param tension Tension value to validate
     * @throw std::invalid_argument if tension is invalid
     */
    static void validateTension(float tension);

    Vector3 p0_, p1_;    // End points
    Vector3 t0_, t1_;    // Tangent vectors
    float tension_;      // Tension parameter
    bool useSimd_;       // Whether to use SIMD operations
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_GEOMETRY_HERMITE_HPP