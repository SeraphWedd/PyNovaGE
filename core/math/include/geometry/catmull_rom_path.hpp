#ifndef PYNOVAGE_MATH_GEOMETRY_CATMULL_ROM_PATH_HPP
#define PYNOVAGE_MATH_GEOMETRY_CATMULL_ROM_PATH_HPP

#include "path.hpp"
#include "catmull_rom.hpp"
#include "../quaternion.hpp"
#include <algorithm>
#include <vector>

namespace pynovage {
namespace math {

class CatmullRomPath : public Path {
public:
    explicit CatmullRomPath(
        const std::vector<Vector3>& points, 
        MovementMode mode = MovementMode::ConstantSpeed);

    State getState(float time) const override;
    State getStateAtDistance(float distance) const override;
    State updateConstantSpeed(const State& currentState, float deltaTime) const override;
    std::unique_ptr<Path> blend(const Path& other, float blendFactor) const override;
    State getClosestPoint(const Vector3& point) const override;
    float getLength() const override;
    float getCurvature(float time) const override;
    bool isClosed() const override;

    // Catmull-Rom specific methods
    void setTension(float tension);
    enum class ParameterizationType {
        Uniform,
        Centripetal,
        Chordal
    };

    void setParameterization(CatmullRom::Parameterization type);

    // Static helpers
    static Vector3 lerp(const Vector3& a, const Vector3& b, float t) {
        return Vector3::lerp(a, b, t);
    }
    void addPoint(const Vector3& point);
    void removePoint(size_t index);

protected:
    void buildArcLengthTable() override;
    float timeToArcLength(float time) const override;
    float arcLengthToTime(float arcLength) const override;

private:
    static constexpr size_t LOOKUP_TABLE_SIZE = 1000;
    static constexpr float DEFAULT_TENSION = 0.0f;

    State computeStateAtParameter(float t) const;
    void updateSpline();
    float findClosestParameter(const Vector3& point) const;
    void resamplePath(std::vector<Vector3>& points, size_t targetCount) const;

    CatmullRom spline_;
    float tension_{DEFAULT_TENSION};
    bool isDirty_{true};
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_GEOMETRY_CATMULL_ROM_PATH_HPP
