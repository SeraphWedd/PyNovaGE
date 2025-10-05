#include "../include/matrix4.hpp"
#include <gtest/gtest.h>
#include <sstream>

using namespace pynovage::math;
using namespace pynovage::math::constants;

TEST(Matrix4Tests, DefaultConstructor) {
    Matrix4x4 m;
    // Check identity matrix
    EXPECT_FLOAT_EQ(m.m[0][0], 1.0f);
    EXPECT_FLOAT_EQ(m.m[1][1], 1.0f);
    EXPECT_FLOAT_EQ(m.m[2][2], 1.0f);
    EXPECT_FLOAT_EQ(m.m[3][3], 1.0f);
    // Check zeros
    EXPECT_FLOAT_EQ(m.m[0][1], 0.0f);
    EXPECT_FLOAT_EQ(m.m[0][2], 0.0f);
    EXPECT_FLOAT_EQ(m.m[0][3], 0.0f);
    EXPECT_FLOAT_EQ(m.m[1][0], 0.0f);
}

TEST(Matrix4Tests, TranslationMatrix) {
    Matrix4x4 trans = Matrix4x4::Translation(2.0f, 3.0f, 4.0f);
    Vector3 point(1.0f, 1.0f, 1.0f);
    Vector3 result = trans.transformPoint(point);
    EXPECT_FLOAT_EQ(result.x, 3.0f);  // 1 + 2
    EXPECT_FLOAT_EQ(result.y, 4.0f);  // 1 + 3
    EXPECT_FLOAT_EQ(result.z, 5.0f);  // 1 + 4
}

TEST(Matrix4Tests, ScaleMatrix) {
    Matrix4x4 scale = Matrix4x4::Scale(2.0f, 3.0f, 4.0f);
    Vector3 point(1.0f, 1.0f, 1.0f);
    Vector3 result = scale.transformPoint(point);
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
    EXPECT_FLOAT_EQ(result.z, 4.0f);
}

TEST(Matrix4Tests, RotationMatrix) {
    // Test 90-degree rotation around Y axis
    Matrix4x4 rot = Matrix4x4::RotationY(constants::half_pi);
    Vector3 point(1.0f, 0.0f, 0.0f);
    Vector3 result = rot.transformPoint(point);
    EXPECT_NEAR(result.x, 0.0f, 1e-6f);
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
    EXPECT_NEAR(result.z, -1.0f, 1e-6f);
}

TEST(Matrix4Tests, MatrixVectorMultiplication) {
    Matrix4x4 mat = Matrix4x4::Translation(1.0f, 2.0f, 3.0f);
    Vector4 point(1.0f, 1.0f, 1.0f, 1.0f);
    Vector4 result = mat * point;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
    EXPECT_FLOAT_EQ(result.z, 4.0f);
    EXPECT_FLOAT_EQ(result.w, 1.0f);
}

TEST(Matrix4Tests, CompoundMultiplication) {
    Matrix4x4 mat1 = Matrix4x4::Translation(1.0f, 0.0f, 0.0f);
    Matrix4x4 mat2 = Matrix4x4::Translation(0.0f, 1.0f, 0.0f);
    mat1 *= mat2;

    Vector4 point(0.0f, 0.0f, 0.0f, 1.0f);
    Vector4 result = mat1 * point;
    EXPECT_FLOAT_EQ(result.x, 1.0f);
    EXPECT_FLOAT_EQ(result.y, 1.0f);
    EXPECT_FLOAT_EQ(result.z, 0.0f);
    EXPECT_FLOAT_EQ(result.w, 1.0f);
}

TEST(Matrix4Tests, Comparison) {
    Matrix4x4 mat1;
    Matrix4x4 mat2;
    Matrix4x4 mat3 = Matrix4x4::Translation(1.0f, 0.0f, 0.0f);

    EXPECT_TRUE(mat1 == mat2);
    EXPECT_FALSE(mat1 != mat2);
    EXPECT_FALSE(mat1 == mat3);
    EXPECT_TRUE(mat1 != mat3);
}

TEST(Matrix4Tests, ArraySubscript) {
    Matrix4x4 mat;
    mat[0][0] = 2.0f;
    mat[1][1] = 3.0f;

    EXPECT_FLOAT_EQ(mat[0][0], 2.0f);
    EXPECT_FLOAT_EQ(mat[1][1], 3.0f);
    EXPECT_FLOAT_EQ(mat[2][2], 1.0f); // Identity
}

TEST(Matrix4Tests, LookAt) {
    Vector3 eye(0.0f, 0.0f, 5.0f);
    Vector3 target(0.0f, 0.0f, 0.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    Matrix4x4 view = Matrix4x4::LookAt(eye, target, up);
    
    // Eye should transform to origin in view space
    Vector4 eye_h(eye.x, eye.y, eye.z, 1.0f);
    Vector4 at_origin = view * eye_h;
    EXPECT_NEAR(at_origin.x, 0.0f, 1e-6f);
    EXPECT_NEAR(at_origin.y, 0.0f, 1e-6f);
    EXPECT_NEAR(at_origin.z, 0.0f, 1e-6f);
}

TEST(Matrix4Tests, Perspective) {
    float fov = half_pi;
    float aspect = 16.0f/9.0f;
    float near = 0.1f;
    float far = 100.0f;

    Matrix4x4 proj = Matrix4x4::Perspective(fov, aspect, near, far);

    // Test that w' = -z for points (OpenGL-style)
    Vector4 any(0.0f, 0.0f, 2.0f, 1.0f);
    Vector4 r = proj * any;
    EXPECT_NEAR(r.w, -any.z, 1e-5f);
}

TEST(Matrix4Tests, Orthographic) {
    Matrix4x4 ortho = Matrix4x4::Orthographic(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);

    // Test center point
    Vector4 center(0.0f, 0.0f, 0.0f, 1.0f);
    Vector4 result = ortho * center;
    EXPECT_NEAR(result.x, 0.0f, 1e-6f);
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
}

TEST(Matrix4Tests, RotationAxis) {
    Vector3 axis(1.0f, 0.0f, 0.0f);
    float angle = half_pi;

    Matrix4x4 rot = Matrix4x4::RotationAxis(axis, angle);
    Vector4 point(0.0f, 1.0f, 0.0f, 1.0f);

    Vector4 result = rot * point;
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
EXPECT_NEAR(result.z, 1.0f, 1e-6f);
}

TEST(Matrix4Tests, EulerAngles) {
    // 90 degrees around Y axis should transform (0,0,1) to (1,0,0)
    Matrix4x4 rot = Matrix4x4::FromEulerAngles(half_pi, 0.0f, 0.0f);
    Vector4 forward(0.0f, 0.0f, 1.0f, 0.0f);

    Vector4 result = rot * forward;
    EXPECT_NEAR(result.x, 1.0f, 1e-6f);
    EXPECT_NEAR(result.z, 0.0f, 1e-6f);
}

TEST(Matrix4Tests, StringFormatting) {
    Matrix4x4 mat = Matrix4x4::Translation(1.0f, 2.0f, 3.0f);
    std::string str = mat.toString();
    
    // Just verify basic formatting structure
    EXPECT_NE(str.find("["), std::string::npos);
    EXPECT_NE(str.find("]"), std::string::npos);
    EXPECT_NE(str.find(","), std::string::npos);
}

TEST(Matrix4Tests, StreamOperator) {
    Matrix4x4 mat = Matrix4x4::Identity();
    std::stringstream ss;
    ss << mat;
    
    // Verify that something was written
    EXPECT_FALSE(ss.str().empty());
}

TEST(Matrix4Tests, InverseIdentity) {
    Matrix4x4 identity;
    Matrix4x4 inverse;
    EXPECT_TRUE(identity.getInverse(inverse));
    
    // Inverse of identity should be identity
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEAR(inverse[i][j], identity[i][j], 1e-6f);
        }
    }
}

TEST(Matrix4Tests, InverseTranslation) {
    Matrix4x4 trans = Matrix4x4::Translation(2.0f, 3.0f, 4.0f);
    Matrix4x4 inverse;
    EXPECT_TRUE(trans.getInverse(inverse));
    
    // Test that inverse translation negates the components
    EXPECT_NEAR(inverse[0][3], -2.0f, 1e-6f);
    EXPECT_NEAR(inverse[1][3], -3.0f, 1e-6f);
    EXPECT_NEAR(inverse[2][3], -4.0f, 1e-6f);
    
    // Test that trans * inverse = identity
    Matrix4x4 result = trans * inverse;
    Matrix4x4 identity;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEAR(result[i][j], identity[i][j], 1e-6f);
        }
    }
}

TEST(Matrix4Tests, InverseRotation) {
    // Rotation by 45 degrees around Y axis
    float angle = constants::pi / 4.0f;
    Matrix4x4 rot = Matrix4x4::RotationY(angle);
    Matrix4x4 inverse;
    EXPECT_TRUE(rot.getInverse(inverse));
    
    // Inverse rotation should be equivalent to negative angle
    Matrix4x4 negRot = Matrix4x4::RotationY(-angle);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEAR(inverse[i][j], negRot[i][j], 1e-6f);
        }
    }
}

TEST(Matrix4Tests, InverseScale) {
    Matrix4x4 scale = Matrix4x4::Scale(2.0f, 3.0f, 4.0f);
    Matrix4x4 inverse;
    EXPECT_TRUE(scale.getInverse(inverse));
    
    // Inverse scale should be reciprocal
    EXPECT_NEAR(inverse[0][0], 1.0f/2.0f, 1e-6f);
    EXPECT_NEAR(inverse[1][1], 1.0f/3.0f, 1e-6f);
    EXPECT_NEAR(inverse[2][2], 1.0f/4.0f, 1e-6f);
}

TEST(Matrix4Tests, NonInvertibleMatrix) {
    // Create a singular matrix (non-invertible)
    Matrix4x4 singular(
        1.0f, 2.0f, 3.0f, 4.0f,
        2.0f, 4.0f, 6.0f, 8.0f,  // Second row is 2 * first row
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    Matrix4x4 inverse;
    EXPECT_FALSE(singular.getInverse(inverse));
}

TEST(Matrix4Tests, TransformCompositionOrder) {
    // Test that transformations are applied in the correct order
    Matrix4x4 translate = Matrix4x4::Translation(1.0f, 0.0f, 0.0f);
    Matrix4x4 rotate = Matrix4x4::RotationY(constants::half_pi);
    
    Vector4 point(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Translate then rotate
    Vector4 tr = rotate * (translate * point);
    
    // Should move point to (1,0,0) then rotate to (0,0,-1)
    EXPECT_NEAR(tr.x, 0.0f, 1e-6f);
    EXPECT_NEAR(tr.y, 0.0f, 1e-6f);
    EXPECT_NEAR(tr.z, -1.0f, 1e-6f);
    
    // Rotate then translate
    Vector4 rt = translate * (rotate * point);
    
    // Should rotate (0,0,0) to (0,0,0) then translate to (1,0,0)
    EXPECT_NEAR(rt.x, 1.0f, 1e-6f);
    EXPECT_NEAR(rt.y, 0.0f, 1e-6f);
    EXPECT_NEAR(rt.z, 0.0f, 1e-6f);
}

TEST(Matrix4Tests, ProjectionMatrixProperties) {
    float fov = constants::half_pi;
    float aspect = 16.0f/9.0f;
    float near = 0.1f;
    float far = 100.0f;
    
    Matrix4x4 proj = Matrix4x4::Perspective(fov, aspect, near, far);
    
    // Test near plane mapping
    Vector4 nearCenter(0.0f, 0.0f, -near, 1.0f);
    Vector4 result = proj * nearCenter;
    result = result * (1.0f/result.w);  // Perspective divide
    EXPECT_NEAR(result.z, -1.0f, 1e-6f);
    
    // Test far plane mapping
    Vector4 farCenter(0.0f, 0.0f, -far, 1.0f);
    result = proj * farCenter;
    result = result * (1.0f/result.w);
    EXPECT_NEAR(result.z, 1.0f, 1e-6f);
    
    // Test frustum corners
    float tanHalf = std::tan(fov/2);
    Vector4 nearTopRight(near*tanHalf*aspect, near*tanHalf, -near, 1.0f);
    result = proj * nearTopRight;
    result = result * (1.0f/result.w);
    EXPECT_NEAR(result.x, 1.0f, 1e-6f);
    EXPECT_NEAR(result.y, 1.0f, 1e-6f);
}

TEST(Matrix4Tests, OrthographicMatrixProperties) {
    float left = -1.0f;
    float right = 1.0f;
    float bottom = -1.0f;
    float top = 1.0f;
    float near = 0.1f;
    float far = 100.0f;
    
    Matrix4x4 ortho = Matrix4x4::Orthographic(left, right, bottom, top, near, far);
    
    // Test that corners map correctly
    Vector4 nearTopRight(right, top, -near, 1.0f);
    Vector4 result = ortho * nearTopRight;
    EXPECT_NEAR(result.x, 1.0f, 1e-6f);
    EXPECT_NEAR(result.y, 1.0f, 1e-6f);
    
    Vector4 farBottomLeft(left, bottom, -far, 1.0f);
    result = ortho * farBottomLeft;
    EXPECT_NEAR(result.x, -1.0f, 1e-6f);
    EXPECT_NEAR(result.y, -1.0f, 1e-6f);
}

TEST(Matrix4Tests, LookAtEdgeCases) {
    // Test looking along each axis
    Vector3 eye(0.0f, 0.0f, 0.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);
    
    // Look down +Z
    Matrix4x4 lookZ = Matrix4x4::LookAt(eye, Vector3(0.0f, 0.0f, 1.0f), up);
    Vector4 forward(0.0f, 0.0f, 1.0f, 0.0f);
    Vector4 result = lookZ * forward;
    EXPECT_NEAR(result.z, 1.0f, 1e-6f);
    
    // Look down +X
    Matrix4x4 lookX = Matrix4x4::LookAt(eye, Vector3(1.0f, 0.0f, 0.0f), up);
    Vector4 right(1.0f, 0.0f, 0.0f, 0.0f);
    result = lookX * right;
    EXPECT_NEAR(result.z, 1.0f, 1e-6f);
    
    // Test looking straight up (special case for up vector)
    Vector3 eyeUp(0.0f, 0.0f, 0.0f);
    Vector3 targetUp(0.0f, 1.0f, 0.0f);
    Vector3 alternateUp(1.0f, 0.0f, 0.0f);
    Matrix4x4 lookUp = Matrix4x4::LookAt(eyeUp, targetUp, alternateUp);
    Vector4 up4(0.0f, 1.0f, 0.0f, 0.0f);
    result = lookUp * up4;
    EXPECT_NEAR(result.z, 1.0f, 1e-6f);
}

TEST(Matrix4Tests, NumericalStability) {
    // Test numerical stability with very small rotations
    float smallAngle = 1e-5f;
    Matrix4x4 smallRot = Matrix4x4::RotationY(smallAngle);
    Vector4 right(1.0f, 0.0f, 0.0f, 0.0f);
    Vector4 result = smallRot * right;
    EXPECT_NEAR(result.x, std::cos(smallAngle), 1e-6f);
EXPECT_NEAR(result.z, -std::sin(smallAngle), 1e-6f);
    
    // Test stability with very large translations
    float largeTranslation = 1e6f;
    Matrix4x4 largeTrans = Matrix4x4::Translation(largeTranslation, 0.0f, 0.0f);
    Matrix4x4 inverse;
    EXPECT_TRUE(largeTrans.getInverse(inverse));
    Matrix4x4 identity = largeTrans * inverse;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            float expected = (i == j) ? 1.0f : 0.0f;
            EXPECT_NEAR(identity[i][j], expected, 1e-5f);
        }
    }
}

TEST(Matrix4Tests, TransformationIdentities) {
    // Test that rotation of 2π equals identity
    Matrix4x4 fullRotation = Matrix4x4::RotationY(constants::two_pi);
    Matrix4x4 identity;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEAR(fullRotation[i][j], identity[i][j], 1e-5f);
        }
    }
    
    // Test that scale by 1 equals identity
    Matrix4x4 unityScale = Matrix4x4::Scale(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEAR(unityScale[i][j], identity[i][j], 1e-6f);
        }
    }
    
    // Test that zero translation equals identity
    Matrix4x4 zeroTranslation = Matrix4x4::Translation(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEAR(zeroTranslation[i][j], identity[i][j], 1e-6f);
        }
    }
}
