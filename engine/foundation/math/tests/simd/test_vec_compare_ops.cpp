#include <gtest/gtest.h>
#include "simd/vector_ops.hpp"

using namespace pynovage::foundation::simd;

class VecCompareOpsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test vectors
        v1 = float4(1.0f, 2.0f, 3.0f, 4.0f);
        v2 = float4(2.0f, 2.0f, 1.0f, 4.0f);
        v3 = float4(1.0f, 2.0f, 3.0f, 4.0f);  // Equal to v1
        zeros = float4(0.0f, 0.0f, 0.0f, 0.0f);
        ones = float4(1.0f, 1.0f, 1.0f, 1.0f);

        // Initialize batch test data
        for (int i = 0; i < 16; ++i) {
            batch_data1[i] = static_cast<float>(i);
            batch_data2[i] = static_cast<float>(15 - i);  // Reverse order
        }
    }

    float4 v1, v2, v3, zeros, ones;
    float batch_data1[16], batch_data2[16];

    void ExpectMask(const int* result, bool x, bool y, bool z, bool w) {
        EXPECT_EQ(result[0] != 0, x);
        EXPECT_EQ(result[1] != 0, y);
        EXPECT_EQ(result[2] != 0, z);
        EXPECT_EQ(result[3] != 0, w);
    }
};

TEST_F(VecCompareOpsTest, LessThan) {
    int result[4];
    VecCompareOps::less_than(v1, v2, result);
    ExpectMask(result, true, false, false, false);  // 1<2, 2=2, 3>1, 4=4
}

TEST_F(VecCompareOpsTest, LessEqual) {
    int result[4];
    VecCompareOps::less_equal(v1, v2, result);
    ExpectMask(result, true, true, false, true);  // 1<2, 2=2, 3>1, 4=4
}

TEST_F(VecCompareOpsTest, GreaterThan) {
    int result[4];
    VecCompareOps::greater_than(v1, v2, result);
    ExpectMask(result, false, false, true, false);  // 1<2, 2=2, 3>1, 4=4
}

TEST_F(VecCompareOpsTest, GreaterEqual) {
    int result[4];
    VecCompareOps::greater_equal(v1, v2, result);
    ExpectMask(result, false, true, true, true);  // 1<2, 2=2, 3>1, 4=4
}

TEST_F(VecCompareOpsTest, Equal) {
    int result[4];
    VecCompareOps::equal(v1, v3, result);
    ExpectMask(result, true, true, true, true);  // v1 == v3
    
    VecCompareOps::equal(v1, v2, result);
    ExpectMask(result, false, true, false, true);  // 1!=2, 2=2, 3!=1, 4=4
}

TEST_F(VecCompareOpsTest, NotEqual) {
    int result[4];
    VecCompareOps::not_equal(v1, v3, result);
    ExpectMask(result, false, false, false, false);  // v1 == v3
    
    VecCompareOps::not_equal(v1, v2, result);
    ExpectMask(result, true, false, true, false);  // 1!=2, 2=2, 3!=1, 4=4
}

TEST_F(VecCompareOpsTest, BatchLessThan) {
    float16 a = float16::load(batch_data1);  // 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    float16 b = float16::load(batch_data2);  // 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    int result[16];
    
    VecCompareOps::less_than_batch4(a, b, result);
    
    // Each group of 4 values (vec4) should have a specific pattern
    for (int i = 0; i < 4; ++i) {
        int base = i * 4;
        for (int j = 0; j < 4; ++j) {
            bool should_be_less = batch_data1[base + j] < batch_data2[base + j];
            EXPECT_EQ(result[base + j] != 0, should_be_less)
                << "Failed at vector " << i << ", component " << j;
        }
    }
}

TEST_F(VecCompareOpsTest, BatchEqual) {
    // Create data where some vectors are equal
    float equal_data[16] = {
        1.0f, 2.0f, 3.0f, 4.0f,    // Equal to next vector
        1.0f, 2.0f, 3.0f, 4.0f,    // Equal to previous vector
        5.0f, 6.0f, 7.0f, 8.0f,    // Different
        1.0f, 2.0f, 3.0f, 4.0f     // Equal to first two vectors
    };
    
    float16 a = float16::load(equal_data);
    float16 b = float16::load(equal_data);  // Compare with itself
    int result[16];
    
    VecCompareOps::equal_batch4(a, b, result);
    
    // All components should be equal
    for (int i = 0; i < 16; ++i) {
        EXPECT_NE(result[i], 0) << "Failed equality at index " << i;
    }
    
    // Modify one component and test again
    equal_data[6] = 99.0f;  // Modify one component in second vector
    float16 c = float16::load(equal_data);
    
    VecCompareOps::equal_batch4(a, c, result);
    
    // Check that only the modified component shows inequality
    for (int i = 0; i < 16; ++i) {
        if (i == 6) {
            EXPECT_EQ(result[i], 0) << "Modified component should not be equal";
        } else {
            EXPECT_NE(result[i], 0) << "Unmodified component should be equal";
        }
    }
}

TEST_F(VecCompareOpsTest, BatchGreaterEqual) {
    float16 a = float16::load(batch_data1);
    float16 b = float16::load(batch_data2);
    int result[16];
    
    VecCompareOps::greater_equal_batch4(a, b, result);
    
    // Test each component
    for (int i = 0; i < 16; ++i) {
        bool should_be_ge = batch_data1[i] >= batch_data2[i];
        EXPECT_EQ(result[i] != 0, should_be_ge)
            << "Failed greater-equal at index " << i;
    }
    
    // Test with equal values
    float equal_data[16];
    for (int i = 0; i < 16; ++i) {
        equal_data[i] = 1.0f;
    }
    
    float16 c = float16::load(equal_data);
    float16 d = float16::load(equal_data);
    
    VecCompareOps::greater_equal_batch4(c, d, result);
    
    // All components should be greater-equal (equal in this case)
    for (int i = 0; i < 16; ++i) {
        EXPECT_NE(result[i], 0) << "Failed equality case at index " << i;
    }
}