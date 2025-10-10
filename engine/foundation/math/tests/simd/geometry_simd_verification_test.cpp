#include <gtest/gtest.h>
#include "simd/geometry_ops.hpp"
#include <random>
#include <vector>

using namespace PyNovaGE::SIMD;

/**
 * @brief Comprehensive SIMD verification tests to ensure SIMD and scalar implementations
 * produce identical results, specifically for the 2D physics use case.
 */
class GeometrySIMDVerificationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize random number generator with fixed seed for reproducibility
        gen.seed(12345);
        uniform_dist = std::uniform_real_distribution<float>(-100.0f, 100.0f);
        size_dist = std::uniform_real_distribution<float>(0.1f, 50.0f);
    }

    // Generate random 3D vector with Z=0 for 2D physics simulation
    Vector<float, 3> generateRandom2DVector() {
        return Vector<float, 3>(uniform_dist(gen), uniform_dist(gen), 0.0f);
    }
    
    // Generate random size vector
    Vector<float, 3> generateRandomSizeVector() {
        return Vector<float, 3>(size_dist(gen), size_dist(gen), 0.0f);
    }

    // Generate random AABB for 2D physics (Z extent is 0)
    AABB<float> generateRandom2DAABB() {
        Vector<float, 3> center = generateRandom2DVector();
        Vector<float, 3> extent = generateRandomSizeVector();
        return AABB<float>(center - extent, center + extent);
    }

    std::mt19937 gen;
    std::uniform_real_distribution<float> uniform_dist;
    std::uniform_real_distribution<float> size_dist;
};

// Scalar implementation of AABB contains for verification
bool scalarAABBContains(const AABB<float>& aabb, const Vector<float, 3>& point) {
    return point[0] >= aabb.min[0] && point[0] <= aabb.max[0] &&
           point[1] >= aabb.min[1] && point[1] <= aabb.max[1] &&
           point[2] >= aabb.min[2] && point[2] <= aabb.max[2];
}

// Scalar implementation of AABB intersection for verification
bool scalarAABBIntersects(const AABB<float>& aabb1, const AABB<float>& aabb2) {
    return aabb1.min[0] <= aabb2.max[0] && aabb1.max[0] >= aabb2.min[0] &&
           aabb1.min[1] <= aabb2.max[1] && aabb1.max[1] >= aabb2.min[1] &&
           aabb1.min[2] <= aabb2.max[2] && aabb1.max[2] >= aabb2.min[2];
}

TEST_F(GeometrySIMDVerificationTest, AABBContainsConsistencyTest) {
    const int NUM_TESTS = 10000;
    
    for (int i = 0; i < NUM_TESTS; ++i) {
        AABB<float> aabb = generateRandom2DAABB();
        Vector<float, 3> point = generateRandom2DVector();
        
        bool simd_result = aabb.contains(point);
        bool scalar_result = scalarAABBContains(aabb, point);
        
        EXPECT_EQ(simd_result, scalar_result) 
            << "SIMD and scalar AABB contains disagree at test " << i
            << "\nAABB min: (" << aabb.min[0] << ", " << aabb.min[1] << ", " << aabb.min[2] << ")"
            << "\nAABB max: (" << aabb.max[0] << ", " << aabb.max[1] << ", " << aabb.max[2] << ")"
            << "\nPoint: (" << point[0] << ", " << point[1] << ", " << point[2] << ")"
            << "\nSIMD result: " << simd_result
            << "\nScalar result: " << scalar_result;
    }
}

TEST_F(GeometrySIMDVerificationTest, AABBIntersectionConsistencyTest) {
    const int NUM_TESTS = 10000;
    
    for (int i = 0; i < NUM_TESTS; ++i) {
        AABB<float> aabb1 = generateRandom2DAABB();
        AABB<float> aabb2 = generateRandom2DAABB();
        
        bool simd_result = aabb1.intersects(aabb2);
        bool scalar_result = scalarAABBIntersects(aabb1, aabb2);
        
        EXPECT_EQ(simd_result, scalar_result) 
            << "SIMD and scalar AABB intersection disagree at test " << i
            << "\nAABB1 min: (" << aabb1.min[0] << ", " << aabb1.min[1] << ", " << aabb1.min[2] << ")"
            << "\nAABB1 max: (" << aabb1.max[0] << ", " << aabb1.max[1] << ", " << aabb1.max[2] << ")"
            << "\nAABB2 min: (" << aabb2.min[0] << ", " << aabb2.min[1] << ", " << aabb2.min[2] << ")"
            << "\nAABB2 max: (" << aabb2.max[0] << ", " << aabb2.max[1] << ", " << aabb2.max[2] << ")"
            << "\nSIMD result: " << simd_result
            << "\nScalar result: " << scalar_result;
    }
}

TEST_F(GeometrySIMDVerificationTest, EdgeCaseAABBContains) {
    AABB<float> aabb(Vector<float, 3>(-1.0f, -1.0f, 0.0f), Vector<float, 3>(1.0f, 1.0f, 0.0f));
    
    // Test exact boundary conditions
    std::vector<std::pair<Vector<float, 3>, bool>> test_cases = {
        {Vector<float, 3>(-1.0f, -1.0f, 0.0f), true},   // min corner
        {Vector<float, 3>(1.0f, 1.0f, 0.0f), true},     // max corner
        {Vector<float, 3>(0.0f, 0.0f, 0.0f), true},     // center
        {Vector<float, 3>(-1.1f, 0.0f, 0.0f), false},   // outside left
        {Vector<float, 3>(1.1f, 0.0f, 0.0f), false},    // outside right
        {Vector<float, 3>(0.0f, -1.1f, 0.0f), false},   // outside bottom
        {Vector<float, 3>(0.0f, 1.1f, 0.0f), false},    // outside top
        {Vector<float, 3>(0.0f, 0.0f, 0.1f), false},    // outside in Z (should be false for 2D)
    };
    
    for (const auto& [point, expected] : test_cases) {
        bool simd_result = aabb.contains(point);
        bool scalar_result = scalarAABBContains(aabb, point);
        
        EXPECT_EQ(simd_result, scalar_result) 
            << "SIMD and scalar disagree for point (" << point[0] << ", " << point[1] << ", " << point[2] << ")";
        EXPECT_EQ(simd_result, expected) 
            << "SIMD result incorrect for point (" << point[0] << ", " << point[1] << ", " << point[2] << ")";
    }
}

TEST_F(GeometrySIMDVerificationTest, EdgeCaseAABBIntersection) {
    // Test various intersection scenarios
    AABB<float> aabb1(Vector<float, 3>(-1.0f, -1.0f, 0.0f), Vector<float, 3>(1.0f, 1.0f, 0.0f));
    
    std::vector<std::pair<AABB<float>, bool>> test_cases = {
        // Identical AABB
        {AABB<float>(Vector<float, 3>(-1.0f, -1.0f, 0.0f), Vector<float, 3>(1.0f, 1.0f, 0.0f)), true},
        // Complete overlap
        {AABB<float>(Vector<float, 3>(-0.5f, -0.5f, 0.0f), Vector<float, 3>(0.5f, 0.5f, 0.0f)), true},
        // Partial overlap
        {AABB<float>(Vector<float, 3>(0.5f, 0.5f, 0.0f), Vector<float, 3>(2.0f, 2.0f, 0.0f)), true},
        // Edge touching
        {AABB<float>(Vector<float, 3>(1.0f, 0.0f, 0.0f), Vector<float, 3>(2.0f, 1.0f, 0.0f)), true},
        // No intersection
        {AABB<float>(Vector<float, 3>(2.0f, 2.0f, 0.0f), Vector<float, 3>(3.0f, 3.0f, 0.0f)), false},
        // Z-axis separation (should not intersect in 3D, but Z is always 0 in 2D physics)
        {AABB<float>(Vector<float, 3>(-1.0f, -1.0f, 1.0f), Vector<float, 3>(1.0f, 1.0f, 2.0f)), false},
    };
    
    for (size_t i = 0; i < test_cases.size(); ++i) {
        const auto& [aabb2, expected] = test_cases[i];
        bool simd_result = aabb1.intersects(aabb2);
        bool scalar_result = scalarAABBIntersects(aabb1, aabb2);
        
        EXPECT_EQ(simd_result, scalar_result) 
            << "SIMD and scalar disagree for test case " << i;
        EXPECT_EQ(simd_result, expected) 
            << "SIMD result incorrect for test case " << i;
    }
}

TEST_F(GeometrySIMDVerificationTest, ZeroDimensionAABB) {
    // Test AABBs with zero dimensions (degenerate cases)
    AABB<float> point_aabb(Vector<float, 3>(5.0f, 5.0f, 0.0f), Vector<float, 3>(5.0f, 5.0f, 0.0f));
    AABB<float> line_aabb(Vector<float, 3>(0.0f, 0.0f, 0.0f), Vector<float, 3>(10.0f, 0.0f, 0.0f));
    
    // Test point containment
    bool contains_exact = point_aabb.contains(Vector<float, 3>(5.0f, 5.0f, 0.0f));
    bool contains_near = point_aabb.contains(Vector<float, 3>(5.01f, 5.0f, 0.0f));
    
    EXPECT_TRUE(contains_exact);
    EXPECT_FALSE(contains_near);
    
    // Test line intersection with point
    bool intersects = point_aabb.intersects(line_aabb);
    bool expected_intersects = scalarAABBIntersects(point_aabb, line_aabb);
    
    EXPECT_EQ(intersects, expected_intersects);
}

TEST_F(GeometrySIMDVerificationTest, PhysicsUseCaseSimulation) {
    // Simulate the actual 2D physics use case where shapes are converted to 3D AABBs with Z=0
    const int NUM_RECTANGLES = 100;
    const int NUM_POINTS = 1000;
    
    std::vector<AABB<float>> rectangles;
    std::vector<Vector<float, 3>> points;
    
    // Generate rectangles (2D shapes converted to 3D AABBs)
    for (int i = 0; i < NUM_RECTANGLES; ++i) {
        Vector<float, 3> center(uniform_dist(gen), uniform_dist(gen), 0.0f);
        Vector<float, 3> half_size(size_dist(gen), size_dist(gen), 0.0f);
        rectangles.emplace_back(center - half_size, center + half_size);
    }
    
    // Generate test points (2D points converted to 3D with Z=0)
    for (int i = 0; i < NUM_POINTS; ++i) {
        points.emplace_back(uniform_dist(gen), uniform_dist(gen), 0.0f);
    }
    
    // Test all rectangle-point containment combinations
    for (size_t r = 0; r < rectangles.size(); ++r) {
        for (size_t p = 0; p < points.size(); ++p) {
            bool simd_result = rectangles[r].contains(points[p]);
            bool scalar_result = scalarAABBContains(rectangles[r], points[p]);
            
            ASSERT_EQ(simd_result, scalar_result) 
                << "Physics simulation containment test failed at rectangle " << r << ", point " << p;
        }
    }
    
    // Test all rectangle-rectangle intersection combinations
    for (size_t i = 0; i < rectangles.size(); ++i) {
        for (size_t j = i + 1; j < rectangles.size(); ++j) {
            bool simd_result = rectangles[i].intersects(rectangles[j]);
            bool scalar_result = scalarAABBIntersects(rectangles[i], rectangles[j]);
            
            ASSERT_EQ(simd_result, scalar_result) 
                << "Physics simulation intersection test failed at rectangles " << i << ", " << j;
        }
    }
}

TEST_F(GeometrySIMDVerificationTest, BitmaskerrorDetection) {
    // This test specifically targets potential bitmask errors mentioned in the conversation summary
    // by testing cases where different bits might be set in the AVX2 result
    
    AABB<float> aabb(Vector<float, 3>(-1.0f, -1.0f, 0.0f), Vector<float, 3>(1.0f, 1.0f, 0.0f));
    
    // Test points that might cause different bitmask patterns
    std::vector<Vector<float, 3>> test_points = {
        // Points that should satisfy all conditions (all bits set)
        Vector<float, 3>(0.0f, 0.0f, 0.0f),     // center
        Vector<float, 3>(-0.5f, -0.5f, 0.0f),   // inside
        
        // Points that fail on X only (bit 0 clear)
        Vector<float, 3>(-2.0f, 0.0f, 0.0f),    // left of AABB
        Vector<float, 3>(2.0f, 0.0f, 0.0f),     // right of AABB
        
        // Points that fail on Y only (bit 1 clear)
        Vector<float, 3>(0.0f, -2.0f, 0.0f),    // below AABB
        Vector<float, 3>(0.0f, 2.0f, 0.0f),     // above AABB
        
        // Points that fail on Z only (bit 2 clear) - important for 2D physics
        Vector<float, 3>(0.0f, 0.0f, 1.0f),     // in front of AABB
        Vector<float, 3>(0.0f, 0.0f, -1.0f),    // behind AABB
        
        // Points that fail multiple conditions
        Vector<float, 3>(-2.0f, -2.0f, 0.0f),   // fail X and Y
        Vector<float, 3>(-2.0f, 0.0f, 1.0f),    // fail X and Z
        Vector<float, 3>(0.0f, -2.0f, 1.0f),    // fail Y and Z
        Vector<float, 3>(-2.0f, -2.0f, 1.0f),   // fail all
    };
    
    for (size_t i = 0; i < test_points.size(); ++i) {
        bool simd_result = aabb.contains(test_points[i]);
        bool scalar_result = scalarAABBContains(aabb, test_points[i]);
        
        EXPECT_EQ(simd_result, scalar_result) 
            << "Bitmask error detected at test point " << i 
            << " (" << test_points[i][0] << ", " << test_points[i][1] << ", " << test_points[i][2] << ")"
            << "\nSIMD result: " << simd_result
            << "\nScalar result: " << scalar_result;
    }
}