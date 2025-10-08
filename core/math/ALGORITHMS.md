# PyNovaGE Math Library Algorithms

This document describes the key algorithms used in the PyNovaGE math library, their implementations,
and mathematical foundations.

## Table of Contents
- [B-Spline Algorithms](#b-spline-algorithms)
  - [Degree Elevation](#degree-elevation)

## B-Spline Algorithms

### Degree Elevation

Elevates a B-spline curve from degree p to p+1 while preserving its exact shape.

#### Algorithm Steps

1. **Bezierization Phase**
   - Insert knots until each interior knot has multiplicity p
   - This splits the spline into a sequence of Bézier segments
   - End knots already have multiplicity p+1 in clamped form

2. **Elevation of Bézier Segments**
   For each Bézier segment with control points P[0]..P[p]:
   - Compute new control points Q[0]..Q[p+1] using:
     ```
     Q[0] = P[0]
     Q[p+1] = P[p]
     For i = 1..p:
       Q[i] = (i/(p+1)) * P[i-1] + (1 - i/(p+1)) * P[i]
     ```

3. **Segment Stitching**
   - Concatenate elevated segment control points
   - Merge shared endpoints between segments
   - Build new knot vector:
     - Same unique knot values
     - End multiplicities increase to p+2
     - Interior multiplicities remain p

#### Implementation Notes

- **SIMD Optimization**:
  - Use SIMD for control point computation
  - Process multiple coordinates (x,y,z) simultaneously
  - Keep memory layout aligned for vector operations

- **Numerical Stability**:
  - Compute weights using reciprocal once per segment
  - Avoid accumulated error in weight computation
  - Use stable lerp implementation

- **Special Cases**:
  - Quadratic to cubic (p=2 to p=3) optimization:
    ```
    newPoints[0] = points[0]
    newPoints[n+1] = points[n]
    For i = 1..n:
      alpha = i/3
      newPoints[i] = (1-alpha)*points[i-1] + alpha*points[i]
    ```

#### Performance Characteristics

- Time Complexity:
  - Knot insertion: O(k*n) where k is number of insertions
  - Elevation: O(n) per segment
  - Total: O(k*n + s*n) where s is number of segments

- Space Complexity:
  - Additional control points: O(n)
  - Temporary storage: O(p) per segment

#### Mathematical Foundation

The algorithm is based on the Oslo Algorithm and preserves these B-spline properties:
1. Partition of unity (weights sum to 1)
2. Local support (control points affect limited curve region)
3. Variation diminishing (curve doesn't oscillate more than control polygon)
4. Endpoint interpolation for clamped curves

#### References

1. Piegl, L. and Tiller, W. "The NURBS Book", Springer
2. Farouki, R.T. "The Oslo Algorithm"
3. De Boor, C. "A Practical Guide to Splines"