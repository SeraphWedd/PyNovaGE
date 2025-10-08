#include "../../include/geometry/path.hpp"
#include <cassert>

namespace pynovage {
namespace math {

Path::Path(const std::vector<Vector3>& points, MovementMode mode) 
    : points_(points)
    , mode_(mode)
    , totalLength_(0.0f)
    , closed_(false) {
    
    // Check if path is closed
    if (points_.size() >= 2) {
        closed_ = (points_.front() - points_.back()).lengthSquared() < 1e-6f;
    }
}

} // namespace math
} // namespace pynovage