#ifndef PYNOVAGE_MATH_GEOMETRY_CATMULL_ROM_HPP
#define PYNOVAGE_MATH_GEOMETRY_CATMULL_ROM_HPP

#include "../vector3.hpp"
#include "hermite.hpp"
#include <vector>
#include <algorithm>
#include <cmath>

namespace pynovage {
namespace math {

/**
 * @brief A class implementing Catmull-Rom splines with SIMD optimization
 * 
 * This class provides functionality for creating and evaluating Catmull-Rom splines.
 * Catmull-Rom splines are a type of interpolating spline that automatically compute
 * tangents to create a smooth curve through a sequence of control points.
 *
 * Parameterization Types:
 * - Uniform (alpha = 0.0): Equal spacing between points
 * - Centripetal (alpha = 0.5): Square root of chord length
 * - Chordal (alpha = 1.0): Actual chord length
 *
 * Performance Characteristics:
 * - SIMD-optimized curve evaluation
 * - Cache-friendly point storage
 * - Efficient segment management
 * - Fast point insertion/deletion
 * - Reuses Hermite basis computation
 *
 * Memory Usage:
 * - Dynamic point sequence storage
 * - Efficient segment caching
 * - Minimal temporary allocations
 * - Streaming-friendly design
 *
 * Usage Guidelines:
 * - Use for camera paths through keyframes
 * - Good for smooth animation through points
 * - Automatic tangent computation
 * - Supports point insertion/removal
 * - Configurable continuity
 *
 * Example:
 * @code
 * // Create a Catmull-Rom spline through points
 * std::vector<Vector3> points = {
 *     Vector3(0,0,0), Vector3(1,1,0),
 *     Vector3(2,-1,0), Vector3(3,0,0)
 * };
 * CatmullRom spline(points);
 *
 * // Evaluate at parameter t
 * Vector3 point = spline.evaluate(0.5f);
 *
 * // Add new point
 * spline.addPoint(Vector3(4,1,0));
 * @endcode
 */
class CatmullRom {
public:
    enum class Parameterization {
        Uniform = 0,     // Equal spacing
        Centripetal,     // Square root of chord length
        Chordal         // Actual chord length
    };

    /**
     * @brief Constructs a Catmull-Rom spline from a sequence of points
     * @param points The sequence of points to interpolate through
     * @param param The parameterization type (default: centripetal)
     * @param tension Tension parameter (default: 0.5)
     * @throw std::invalid_argument if fewer than 2 points provided
     */
    explicit CatmullRom(const std::vector<Vector3>& points,
                       Parameterization param = Parameterization::Centripetal,
                       float tension = 0.5f);

    /**
     * @brief Evaluates the spline at parameter t
     * @param t The parameter value in [0,1]
     * @return The point on the curve at parameter t
     */
    Vector3 evaluate(float t) const;

    /**
     * @brief Evaluates multiple points along the spline efficiently
     * @param parameters Vector of parameter values in [0,1]
     * @return Vector of points on the curve
     */
    std::vector<Vector3> evaluateMultiple(const std::vector<float>& parameters) const;

    /**
     * @brief Computes the derivative of the spline
     * @param t The parameter value in [0,1]
     * @return The derivative vector at parameter t
     */
    Vector3 derivative(float t) const;

    /**
     * @brief Sets the parameterization type
     * @param param New parameterization type
     */
    void setParameterization(Parameterization param);

    /**
     * @brief Sets the tension parameter
     * @param tension New tension value (affects curve shape)
     * @throw std::invalid_argument if tension is invalid
     */
    void setTension(float tension);

    /**
     * @brief Adds a point to the end of the spline
     * @param point The new point to add
     */
    void addPoint(const Vector3& point);

    /**
     * @brief Inserts a point at the specified index
     * @param point The point to insert
     * @param index The index to insert at
     * @throw std::out_of_range if index is invalid
     */
    void insertPoint(const Vector3& point, size_t index);

    /**
     * @brief Removes a point at the specified index
     * @param index The index of the point to remove
     * @throw std::out_of_range if index is invalid
     */
    void removePoint(size_t index);

    /**
     * @brief Gets the current tension parameter
     * @return Current tension value
     */
    float getTension() const { return tension_; }

    /**
     * @brief Gets the current parameterization type
     * @return Current parameterization type
     */
    Parameterization getParameterization() const { return param_; }

    /**
     * @brief Gets the sequence of control points
     * @return Vector of control points
     */
    const std::vector<Vector3>& getControlPoints() const { return points_; }

    /**
     * @brief Gets the number of segments in the spline
     * @return Number of segments (n-3 for n points)
     */
    size_t getSegmentCount() const { return points_.size() > 3 ? points_.size() - 3 : 0; }

private:
    /**
     * @brief Computes the tangent at a point using the current parameterization
     * @param prev Previous point
     * @param curr Current point
     * @param next Next point
     * @return The computed tangent vector
     */
    Vector3 computeTangent(const Vector3& prev, const Vector3& curr, const Vector3& next) const;

    /**
     * @brief Computes the parameter value between points based on parameterization
     * @param p0 First point
     * @param p1 Second point
     * @return Parameter value based on current parameterization
     */
    float computeParameter(const Vector3& p0, const Vector3& p1) const;

    /**
     * @brief Gets the Hermite segment for a given index
     * @param index The segment index
     * @return Hermite curve for the segment
     */
    Hermite getSegment(size_t index) const;

    /**
     * @brief Updates segment parameters after point changes
     */
    void updateSegmentParameters();

    /**
     * @brief Validates input points
     * @param points Points to validate
     * @throw std::invalid_argument if validation fails
     */
    static void validatePoints(const std::vector<Vector3>& points);

    std::vector<Vector3> points_;        // Control points
    std::vector<float> parameters_;      // Parameter values at points
    Parameterization param_;             // Parameterization type
    float tension_;                      // Tension parameter
    bool useSimd_;                       // Whether to use SIMD operations
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_GEOMETRY_CATMULL_ROM_HPP