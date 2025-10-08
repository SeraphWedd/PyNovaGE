#ifndef PYNOVAGE_MATH_GEOMETRY_PATH_HPP
#define PYNOVAGE_MATH_GEOMETRY_PATH_HPP

#include "catmull_rom.hpp"
#include "bezier.hpp"
#include "bspline.hpp"
#include "hermite.hpp"
#include "../vector3.hpp"
#include <vector>
#include <memory>
#include <functional>

namespace pynovage {
namespace math {

/**
 * @brief The type of interpolation to use for the path
 */
enum class PathType {
    CatmullRom,    ///< Smooth path through control points
    Bezier,        ///< Precise control with Bézier curves
    BSpline,       ///< Uniform B-spline interpolation
    Linear         ///< Simple linear interpolation between points
};

/**
 * @brief High-level path system for game engine usage
 * 
 * This class provides a unified interface for creating and managing paths
 * in the game engine. It supports different interpolation methods and
 * provides utilities for path following and modification.
 */
class Path {
public:
    /**
     * @brief Creates a path with specified interpolation type
     * @param type The type of path interpolation to use
     */
    explicit Path(PathType type = PathType::CatmullRom);

    /**
     * @brief Adds a control point to the path
     * @param point The point to add
     * @param tangent Optional tangent vector (used for some curve types)
     */
    void addPoint(const Vector3& point, const Vector3& tangent = Vector3::zero());

    /**
     * @brief Inserts a control point at specified index
     * @param point The point to insert
     * @param index Where to insert the point
     * @param tangent Optional tangent vector
     */
    void insertPoint(const Vector3& point, size_t index, const Vector3& tangent = Vector3::zero());

    /**
     * @brief Removes a control point at specified index
     * @param index The index of the point to remove
     */
    void removePoint(size_t index);

    /**
     * @brief Updates a control point's position
     * @param index The index of the point to update
     * @param point The new position
     * @param tangent Optional new tangent vector
     */
    void updatePoint(size_t index, const Vector3& point, const Vector3& tangent = Vector3::zero());

    /**
     * @brief Gets the position at a point along the path
     * @param t Parameter in range [0,1]
     * @return Position vector at parameter t
     */
    Vector3 getPosition(float t) const;

    /**
     * @brief Gets the tangent vector at a point along the path
     * @param t Parameter in range [0,1]
     * @return Tangent vector at parameter t
     */
    Vector3 getTangent(float t) const;

    /**
     * @brief Gets the normal vector at a point along the path
     * @param t Parameter in range [0,1]
     * @param up Reference up vector (default is world up)
     * @return Normal vector at parameter t
     */
    Vector3 getNormal(float t, const Vector3& up = Vector3(0, 1, 0)) const;

    /**
     * @brief Gets binormal vector (cross product of tangent and normal)
     * @param t Parameter in range [0,1]
     * @param up Reference up vector (default is world up)
     * @return Binormal vector at parameter t
     */
    Vector3 getBinormal(float t, const Vector3& up = Vector3(0, 1, 0)) const;

    /**
     * @brief Gets position and orientation frame at point along path
     * @param t Parameter in range [0,1]
     * @param up Reference up vector (default is world up)
     * @return Tuple of (position, tangent, normal, binormal)
     */
    std::tuple<Vector3, Vector3, Vector3, Vector3> 
    getFrame(float t, const Vector3& up = Vector3(0, 1, 0)) const;

    /**
     * @brief Gets the parameter t at a given arc length along the path
     * @param distance Distance along the path
     * @return Parameter t in range [0,1]
     */
    float getParameterAtDistance(float distance) const;

    /**
     * @brief Gets the total length of the path
     * @return Path length
     */
    float getLength() const;

    /**
     * @brief Gets the number of control points in the path
     * @return Number of control points
     */
    size_t getPointCount() const;

    /**
     * @brief Gets a control point by index
     * @param index Index of the point
     * @return The control point position
     */
    Vector3 getPoint(size_t index) const;

    /**
     * @brief Gets a control point's tangent by index
     * @param index Index of the point
     * @return The control point's tangent
     */
    Vector3 getPointTangent(size_t index) const;

    /**
     * @brief Sets the interpolation type
     * @param type New path type
     */
    void setType(PathType type);

    /**
     * @brief Gets current interpolation type
     * @return Current path type
     */
    PathType getType() const { return type_; }

    /**
     * @brief Sets tension parameter (affects curve shape)
     * @param tension New tension value
     */
    void setTension(float tension);

    /**
     * @brief Gets current tension parameter
     * @return Current tension value
     */
    float getTension() const { return tension_; }

    /**
     * @brief Sets if the path should be treated as closed
     * @param closed Whether the path forms a closed loop
     */
    void setClosed(bool closed);

    /**
     * @brief Checks if path is closed
     * @return True if path is closed
     */
    bool isClosed() const { return closed_; }

    /**
     * @brief Updates path data when points change
     * This must be called after making multiple point changes
     */
    void updatePath();

private:
    void rebuildPath();
    void ensurePathExists() const;
    size_t validateIndex(size_t index) const;

    PathType type_;
    float tension_;
    bool closed_;
    
    std::vector<Vector3> points_;
    std::vector<Vector3> tangents_;
    std::vector<float> lengths_;  // Cached segment lengths
    float totalLength_;           // Cached total length

    // Different curve implementations
    std::unique_ptr<CatmullRom> catmullPath_;
    std::unique_ptr<Bezier> bezierPath_;
    std::unique_ptr<BSpline> bsplinePath_;
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_GEOMETRY_PATH_HPP