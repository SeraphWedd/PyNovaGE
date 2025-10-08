#ifndef PYNOVAGE_MATH_GEOMETRY_PATH_HPP
#define PYNOVAGE_MATH_GEOMETRY_PATH_HPP

#include "../vector3.hpp"
#include "../quaternion.hpp"
#include "catmull_rom.hpp"
#include "bezier.hpp"
#include "bspline.hpp"
#include <vector>
#include <memory>

namespace pynovage {
namespace math {

/**
 * @brief Abstract base class for path interpolation
 * 
 * Provides a unified interface for path traversal with:
 * - Constant velocity movement
 * - Orientation control
 * - Path blending
 * - Arc-length parameterization
 * 
 * Performance Characteristics:
 * - O(1) evaluation time for position and orientation
 * - O(log N) time for closest point queries
 * - Cache-friendly data structures
 * - SIMD-optimized calculations
 */
class Path {
public:
    /**
     * @brief Represents the state of an object on the path
     */
    struct State {
        Vector3 position;     // Current position
        Quaternion rotation;  // Current orientation
        float time;          // Current time
        float distance;      // Distance along path
        float speed;         // Current speed
        float curvature;     // Local path curvature
    };

    /**
     * @brief Movement mode for path traversal
     */
    enum class MovementMode {
        ConstantSpeed,     // Maintain constant speed
        ConstantTime,      // Constant time intervals
        VariableSpeed,     // Speed varies with curvature
        CustomSpeed        // User-defined speed function
    };

    /**
     * @brief Creates a path from a sequence of control points
     * @param points Control points defining the path
     * @param mode Movement mode for traversal
     */
    Path(const std::vector<Vector3>& points, MovementMode mode = MovementMode::ConstantSpeed);

    /**
     * @brief Virtual destructor
     */
    virtual ~Path() = default;

    /**
     * @brief Get the state at a given time
     * @param time Time parameter [0,1]
     * @return State containing position and orientation
     */
    virtual State getState(float time) const = 0;

    /**
     * @brief Get the state after moving a distance
     * @param distance Distance to move along path
     * @return State at the given distance
     */
    virtual State getStateAtDistance(float distance) const = 0;

    /**
     * @brief Update state with constant velocity
     * @param currentState Current state to update
     * @param deltaTime Time step
     * @return Updated state
     */
    virtual State updateConstantSpeed(const State& currentState, float deltaTime) const = 0;

    /**
     * @brief Blend between two paths
     * @param other Path to blend with
     * @param blendFactor Blend factor [0,1]
     * @return New path representing the blend
     */
    virtual std::unique_ptr<Path> blend(const Path& other, float blendFactor) const = 0;

    /**
     * @brief Get the closest point on the path
     * @param point Query point
     * @return State at the closest point
     */
    virtual State getClosestPoint(const Vector3& point) const = 0;

    /**
     * @brief Get total path length
     */
    virtual float getLength() const = 0;

    /**
     * @brief Get path curvature at a point
     * @param time Time parameter [0,1]
     */
    virtual float getCurvature(float time) const = 0;

    /**
     * @brief Check if path is closed (loops)
     */
    virtual bool isClosed() const = 0;

protected:
    std::vector<Vector3> points_;        // Control points
    MovementMode mode_;                  // Movement mode
    float totalLength_;                  // Total path length
    bool closed_;                        // Whether path is closed

    // Lookup tables for arc-length parameterization
    std::vector<float> arcLengths_;     // Cumulative arc lengths
    std::vector<float> parameters_;      // Corresponding curve parameters

    /**
     * @brief Build arc-length lookup table
     */
    virtual void buildArcLengthTable() = 0;

    /**
     * @brief Convert time to arc-length parameter
     * @param time Time parameter [0,1]
     * @return Arc-length parameter
     */
    virtual float timeToArcLength(float time) const = 0;

    /**
     * @brief Convert arc-length to time parameter
     * @param arcLength Arc-length parameter
     * @return Time parameter [0,1]
     */
    virtual float arcLengthToTime(float arcLength) const = 0;
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_GEOMETRY_PATH_HPP