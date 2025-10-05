# Math Library Naming Conventions

## Namespace Organization
All math components live under the `pynovage::math` namespace, with specific sub-namespaces:
- `pynovage::math` - Core math functionality (vectors, matrices, quaternions)
- `pynovage::math::geometry` - Geometric primitives and operations
- `pynovage::math::constants` - Mathematical constants

## Class Naming
### Core Classes
- Vector classes: `Vector2`, `Vector3`, `Vector4`
- Matrix classes: `Matrix2`, `Matrix3`, `Matrix4` (changing from Matrix3x3/Matrix4x4)
- Quaternion class: `Quaternion`

### Geometric Classes
- All geometric primitives start with uppercase: `Sphere`, `AABB`, `Plane`, etc.
- All utility classes should be descriptive: `BroadPhase`, `CollisionResponse`

## Method Naming

### Static Factory Methods
- All lowercase: `identity()`, `zero()`, `one()`
- Descriptive verbs for transformations: `scale()`, `rotation()`, `translation()`
- Composites use 'from': `fromAxisAngle()`, `fromEulerAngles()`

### Instance Methods
- All lowercase for operations: `normalize()`, `transpose()`
- Non-mutating variants add 'ed': `normalized()`, `transposed()`
- Query methods use 'is' prefix: `isZero()`, `isIdentity()`
- Getters omit 'get' prefix: `length()`, `determinant()`
- Multiple word methods use camelCase: `lengthSquared()`, `dotProduct()`

### Geometric Operations
- Clear verbs for actions: `intersect()`, `contains()`
- Query methods return bool: `intersectsWith()`, `containsPoint()`
- Result methods include 'result': `intersectionResult()`, `collisionResult()`

## Parameter Naming
- Use descriptive lowercase names: `angle`, `radius`, `scale`
- Vectors use descriptive position names: `origin`, `direction`, `target`
- Matrix parameters use descriptive role: `transformation`, `rotation`, `view`
- Boolean flags use 'is' prefix: `isStatic`, `isNormalized`
- Tolerance/epsilon parameters use 'epsilon': `epsilon`, `tolerance`

## Return Values
- Geometric queries return `std::optional<T>` for nullable results
- Operations that can fail return bool and use out parameters
- Vector/matrix operations return new instances unless marked as mutating
- Mathematical results use appropriate numeric types (float/double)

## Documentation
### Required Sections
- Brief description of purpose
- Parameter descriptions with units/ranges
- Return value description
- Performance characteristics/benchmarks
- Usage examples
- Thread safety notes

### Format
```cpp
/**
 * @brief Short description
 * 
 * Detailed description of functionality and behavior.
 * Include mathematical formulas or references if relevant.
 *
 * Performance Characteristics:
 * - O(n) time complexity
 * - Benchmarks: operation ~Xns
 * - SIMD optimized: Yes/No
 *
 * @param name Description [units/range]
 * @return Description of return value
 *
 * Example:
 * @code
 * // Example code
 * @endcode
 */
```

## Error Handling
- Use `std::optional` for operations that may not produce a result
- Use validation methods prefixed with 'validate': `validateInputs()`
- Throw `std::invalid_argument` for invalid parameters
- Throw `std::out_of_range` for index/range violations
- Return bool for operations that can fail, use out parameters for results

## Constant Naming
- Mathematical constants: lowercase with underscores: `pi`, `two_pi`, `half_pi`
- Directional constants: lowercase methods: `up()`, `down()`, `left()`, `right()`
- Template parameters: PascalCase: `T`, `Size`, `Dimensions`

## File Organization
### Header Files
- Core math: directly in `include/`
- Geometry: in `include/geometry/`
- Implementations: matching structure in `src/`
- Tests: matching structure in `tests/`

### Implementation Order
1. Constructors and static factories
2. Basic operations
3. Geometric/mathematical operations
4. Utility functions
5. Static constants
6. Stream operators