#include <gtest/gtest.h>
#include "simd/types.hpp"
#include <array>

using namespace pynovage::foundation::simd;

class SimdTypesTest : public ::testing::Test {
protected:
    const float test_data[16] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    };
    
    // For aligned tests
    alignas(64) float aligned_data[16] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    };
};

TEST_F(SimdTypesTest, Float4Construction) {
    // Test default constructor
    float4 v1;
    EXPECT_EQ(sizeof(v1), sizeof(float) * 4);
    
    // Test explicit constructor
    float4 v2(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_EQ(v2[0], 1.0f);
    EXPECT_EQ(v2[1], 2.0f);
    EXPECT_EQ(v2[2], 3.0f);
    EXPECT_EQ(v2[3], 4.0f);
}

TEST_F(SimdTypesTest, Float4LoadStore) {
    // Test unaligned load
    float4 v1 = float4::load(test_data);
    EXPECT_EQ(v1[0], 1.0f);
    EXPECT_EQ(v1[1], 2.0f);
    EXPECT_EQ(v1[2], 3.0f);
    EXPECT_EQ(v1[3], 4.0f);

    // Test aligned load
    float4 v2 = float4::load_aligned(aligned_data);
    EXPECT_EQ(v2[0], 1.0f);
    EXPECT_EQ(v2[1], 2.0f);
    EXPECT_EQ(v2[2], 3.0f);
    EXPECT_EQ(v2[3], 4.0f);

    // Test unaligned store
    float result[4];
    v1.store(result);
    EXPECT_EQ(result[0], 1.0f);
    EXPECT_EQ(result[1], 2.0f);
    EXPECT_EQ(result[2], 3.0f);
    EXPECT_EQ(result[3], 4.0f);

    // Test aligned store
    alignas(16) float aligned_result[4];
    v1.store_aligned(aligned_result);
    EXPECT_EQ(aligned_result[0], 1.0f);
    EXPECT_EQ(aligned_result[1], 2.0f);
    EXPECT_EQ(aligned_result[2], 3.0f);
    EXPECT_EQ(aligned_result[3], 4.0f);
}

TEST_F(SimdTypesTest, Float4Broadcast) {
    float4 v = float4::broadcast(42.0f);
    EXPECT_EQ(v[0], 42.0f);
    EXPECT_EQ(v[1], 42.0f);
    EXPECT_EQ(v[2], 42.0f);
    EXPECT_EQ(v[3], 42.0f);
}

TEST_F(SimdTypesTest, Float8Construction) {
    // Test default constructor
    float8 v1;
    EXPECT_EQ(sizeof(v1), sizeof(float) * 8);
    
    // Test explicit constructor
    float8 v2(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(v2[i], static_cast<float>(i + 1));
    }
}

TEST_F(SimdTypesTest, Float8LoadStore) {
    // Test unaligned load
    float8 v1 = float8::load(test_data);
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(v1[i], test_data[i]);
    }

    // Test aligned load
    float8 v2 = float8::load_aligned(aligned_data);
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(v2[i], aligned_data[i]);
    }

    // Test unaligned store
    float result[8];
    v1.store(result);
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(result[i], test_data[i]);
    }

    // Test aligned store
    alignas(32) float aligned_result[8];
    v1.store_aligned(aligned_result);
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(aligned_result[i], test_data[i]);
    }
}

TEST_F(SimdTypesTest, Float8Broadcast) {
    float8 v = float8::broadcast(42.0f);
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(v[i], 42.0f);
    }
}

TEST_F(SimdTypesTest, Float16Construction) {
    // Test default constructor
    float16 v1;
    EXPECT_EQ(sizeof(v1), sizeof(float) * 16);
    
    // Test explicit constructor
    float16 v2(
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    );
    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(v2[i], static_cast<float>(i + 1));
    }
}

TEST_F(SimdTypesTest, Float16LoadStore) {
    // Test unaligned load
    float16 v1 = float16::load(test_data);
    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(v1[i], test_data[i]);
    }

    // Test aligned load
    float16 v2 = float16::load_aligned(aligned_data);
    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(v2[i], aligned_data[i]);
    }

    // Test unaligned store
    float result[16];
    v1.store(result);
    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(result[i], test_data[i]);
    }

    // Test aligned store
    alignas(64) float aligned_result[16];
    v1.store_aligned(aligned_result);
    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(aligned_result[i], test_data[i]);
    }
}

TEST_F(SimdTypesTest, Float16Broadcast) {
    float16 v = float16::broadcast(42.0f);
    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(v[i], 42.0f);
    }
}

TEST_F(SimdTypesTest, AlignmentRequirements) {
    // Test alignment requirements
    EXPECT_EQ(alignof(float4), 16);
    EXPECT_EQ(alignof(float8), 32);
    EXPECT_EQ(alignof(float16), 64);
}

TEST_F(SimdTypesTest, SizeRequirements) {
    // Test size requirements
    EXPECT_EQ(sizeof(float4), sizeof(float) * 4);
    EXPECT_EQ(sizeof(float8), sizeof(float) * 8);
    EXPECT_EQ(sizeof(float16), sizeof(float) * 16);
}