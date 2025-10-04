#include "vector.hpp"
#include "linear_allocator.hpp"
#include <gtest/gtest.h>
#include <string>
#include <memory>

using namespace pynovage::memory;

// Test fixture for Vector tests
class VectorTest : public ::testing::Test {
protected:
    static constexpr std::size_t kDefaultAlignment = 16;
    std::unique_ptr<LinearAllocator<kDefaultAlignment>> allocator_;
    
    void SetUp() override {
        allocator_ = std::make_unique<LinearAllocator<kDefaultAlignment>>(1024 * 1024);
    }
    
    void TearDown() override {
        allocator_.reset();
    }
};

TEST_F(VectorTest, DefaultConstructor) {
    Vector<int> v;
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0);
}

TEST_F(VectorTest, PushBackSmallBuffer) {
    Vector<int, 4> v;
    
    for (int i = 0; i < 4; ++i) {
        v.push_back(i);
    }
    
    EXPECT_EQ(v.size(), 4);
    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(v[i], i);
    }
}

TEST_F(VectorTest, PushBackHeapAllocation) {
    Vector<int, 4> v(allocator_.get());
    
    // Push more items than small buffer can hold
    for (int i = 0; i < 8; ++i) {
        v.push_back(i);
    }
    
    EXPECT_EQ(v.size(), 8);
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(v[i], i);
    }
}

TEST_F(VectorTest, Reserve) {
    Vector<int> v(allocator_.get());
    
    v.reserve(100);
    EXPECT_GE(v.capacity(), 100);
    EXPECT_EQ(v.size(), 0);
    
    // Make sure we can still add elements
    for (int i = 0; i < 10; ++i) {
        v.push_back(i);
    }
    
    EXPECT_EQ(v.size(), 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(v[i], i);
    }
}

TEST_F(VectorTest, SmallBufferOptimization) {
    Vector<int, 4> small(allocator_.get());
    Vector<int, 32> large(allocator_.get());
    
    // Small vector should use SBO
    for (int i = 0; i < 4; ++i) {
        small.push_back(i);
    }
    
    // Large vector should not use SBO (sizeof(int) * 32 > 64 bytes)
    for (int i = 0; i < 32; ++i) {
        large.push_back(i);
    }
    
    EXPECT_EQ(small.size(), 4);
    EXPECT_EQ(large.size(), 32);
}

TEST_F(VectorTest, CopyConstructor) {
    Vector<int, 4> v(allocator_.get());
    v.push_back(1);
    v.push_back(2);
    
    Vector<int, 4> copy(v);
    EXPECT_EQ(copy.size(), 2);
    EXPECT_EQ(copy[0], 1);
    EXPECT_EQ(copy[1], 2);
}

TEST_F(VectorTest, MoveConstructor) {
    Vector<int, 4> v(allocator_.get());
    v.push_back(1);
    v.push_back(2);
    
    Vector<int, 4> moved(std::move(v));
    EXPECT_EQ(moved.size(), 2);
    EXPECT_EQ(moved[0], 1);
    EXPECT_EQ(moved[1], 2);
    EXPECT_TRUE(v.empty());
}

TEST_F(VectorTest, Insert) {
    Vector<int, 4> v;
    for (int i = 0; i < 3; ++i) {
        v.push_back(i); // [0, 1, 2]
    }
    
    auto it = v.begin() + 1;
    v.insert(it, 100);  // [0, 100, 1, 2]
    
    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v[0], 0);
    EXPECT_EQ(v[1], 100);
    EXPECT_EQ(v[2], 1);
    EXPECT_EQ(v[3], 2);
}

TEST_F(VectorTest, Erase) {
    Vector<int, 4> v;
    for (int i = 0; i < 4; ++i) {
        v.push_back(i); // [0, 1, 2, 3]
    }
    
    auto it = v.begin() + 1;
    v.erase(it);  // [0, 2, 3]
    
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], 0);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

TEST_F(VectorTest, ResizeLarger) {
    Vector<int, 4> v;
    v.push_back(1);
    v.push_back(2);
    
    v.resize(4);
    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    // v[2] and v[3] are value-initialized
}

TEST_F(VectorTest, ResizeSmaller) {
    Vector<int, 4> v;
    for (int i = 0; i < 4; ++i) {
        v.push_back(i);
    }
    
    v.resize(2);
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0], 0);
    EXPECT_EQ(v[1], 1);
}

TEST_F(VectorTest, ShrinkToFit) {
    Vector<int, 4> v(allocator_.get());
    
    // Fill past small buffer size
    for (int i = 0; i < 8; ++i) {
        v.push_back(i);
    }
    
    // Remove some elements and shrink
    v.resize(4);
    auto cap_before = v.capacity();
    v.shrink_to_fit();
    
    EXPECT_TRUE(v.capacity() < cap_before);
    EXPECT_EQ(v.size(), 4);
    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(v[i], i);
    }
}

TEST_F(VectorTest, Clear) {
    Vector<int, 4> v(allocator_.get());
    v.push_back(1);
    v.push_back(2);
    
    v.clear();
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0);
}

TEST_F(VectorTest, EmplaceBack) {
    Vector<int, 4> v(allocator_.get());
    v.emplace_back(1);
    v.emplace_back(2);
    
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
}

TEST_F(VectorTest, PopBack) {
    Vector<int, 4> v;
    v.push_back(1);
    v.push_back(2);
    
    v.pop_back();
    EXPECT_EQ(v.size(), 1);
    EXPECT_EQ(v[0], 1);
}

TEST_F(VectorTest, SwapSmallBuffer) {
    Vector<int, 4> v1;
    Vector<int, 4> v2;
    
    v1.push_back(1);
    v1.push_back(2);
    
    v2.push_back(3);
    v2.push_back(4);
    v2.push_back(5);
    
    v1.swap(v2);
    
    EXPECT_EQ(v1.size(), 3);
    EXPECT_EQ(v2.size(), 2);
    
    EXPECT_EQ(v1[0], 3);
    EXPECT_EQ(v1[1], 4);
    EXPECT_EQ(v1[2], 5);
    
    EXPECT_EQ(v2[0], 1);
    EXPECT_EQ(v2[1], 2);
}