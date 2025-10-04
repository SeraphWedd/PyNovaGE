#include "../include/deque.hpp"
#include <gtest/gtest.h>

namespace pynovage {
namespace memory {
namespace tests {

// Mock allocator for testing
class MockAllocator : public IAllocator {
public:
    void* allocateImpl(std::size_t size, std::size_t /*alignment*/) override {
        // Use matching allocation/deallocation pair to avoid heap issues in tests
        return ::operator new(size);
    }

    void deallocateImpl(void* ptr) override {
        ::operator delete(ptr);
    }

    void reset() override {}
    std::size_t getUsedMemory() const override { return 0; }
    std::size_t getTotalMemory() const override { return 0; }
    std::size_t getAllocationCount() const override { return 0; }
};

TEST(DequeTests, DefaultConstructor) {
    Deque<int> deque;
    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(deque.size(), 0);
}

TEST(DequeTests, AllocatorConstructor) {
    MockAllocator allocator;
    Deque<int> deque(&allocator);
    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(deque.size(), 0);
}

TEST(DequeTests, FrontBackEmpty) {
    MockAllocator allocator;
    Deque<int> deque(&allocator);
    EXPECT_THROW(deque.front(), std::out_of_range);
    EXPECT_THROW(deque.back(), std::out_of_range);
}

TEST(DequeTests, PushPopSingleElement) {
    MockAllocator allocator;
    Deque<int> deque(&allocator);

    // Test push_back
    deque.push_back(42);
    EXPECT_EQ(deque.size(), 1);
    EXPECT_FALSE(deque.empty());
    EXPECT_EQ(deque.front(), 42);
    EXPECT_EQ(deque.back(), 42);

    // Test pop_back
    deque.pop_back();
    EXPECT_EQ(deque.size(), 0);
    EXPECT_TRUE(deque.empty());

    // Test push_front
    deque.push_front(43);
    EXPECT_EQ(deque.size(), 1);
    EXPECT_FALSE(deque.empty());
    EXPECT_EQ(deque.front(), 43);
    EXPECT_EQ(deque.back(), 43);

    // Test pop_front
    deque.pop_front();
    EXPECT_EQ(deque.size(), 0);
    EXPECT_TRUE(deque.empty());
}

TEST(DequeTests, MultiplePushPop) {
    MockAllocator allocator;
    Deque<int> deque(&allocator);

    // Push elements from both ends
    deque.push_back(1);
    deque.push_back(2);
    deque.push_front(0);
    deque.push_front(-1);

    EXPECT_EQ(deque.size(), 4);
    EXPECT_EQ(deque.front(), -1);
    EXPECT_EQ(deque.back(), 2);

    // Pop from back
    deque.pop_back();
    EXPECT_EQ(deque.size(), 3);
    EXPECT_EQ(deque.back(), 1);

    // Pop from front
    deque.pop_front();
    EXPECT_EQ(deque.size(), 2);
    EXPECT_EQ(deque.front(), 0);
}

TEST(DequeTests, PushPopBlockBoundaries) {
    MockAllocator allocator;
    Deque<int> deque(&allocator);

    // Push enough elements to force block allocation
    const int NUM_ELEMENTS = 1000;
    
    // Push alternating front/back
    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        if (i % 2 == 0) {
            deque.push_back(i);
        } else {
            deque.push_front(-i);
        }
    }

    EXPECT_EQ(deque.size(), NUM_ELEMENTS);

    // Pop everything alternating front/back
    int remaining = NUM_ELEMENTS;
    while (remaining > 0) {
        if (remaining % 2 == 0) {
            deque.pop_back();
        } else {
            deque.pop_front();
        }
        remaining--;
    }

    EXPECT_TRUE(deque.empty());
}

TEST(DequeTests, EmptyBehavior) {
    MockAllocator allocator;
    Deque<int> deque(&allocator);

    EXPECT_THROW(deque.pop_front(), std::out_of_range);
    EXPECT_THROW(deque.pop_back(), std::out_of_range);

    deque.push_back(1);
    deque.pop_back();

    EXPECT_THROW(deque.pop_front(), std::out_of_range);
    EXPECT_THROW(deque.pop_back(), std::out_of_range);
}

TEST(DequeTests, EmplaceOperations) {
    MockAllocator allocator;
    Deque<std::pair<int, int>> deque(&allocator);

    // Emplace at back
    deque.emplace_back(1, 2);
    EXPECT_EQ(deque.size(), 1);
    EXPECT_EQ(deque.front().first, 1);
    EXPECT_EQ(deque.front().second, 2);

    // Emplace at front
    deque.emplace_front(0, -1);
    EXPECT_EQ(deque.size(), 2);
    EXPECT_EQ(deque.front().first, 0);
    EXPECT_EQ(deque.front().second, -1);
    EXPECT_EQ(deque.back().first, 1);
    EXPECT_EQ(deque.back().second, 2);
}

TEST(DequeTests, RandomAccess) {
    MockAllocator allocator;
    Deque<int> deque(&allocator);

    // Fill with some elements
    for (int i = 0; i < 10; ++i) {
        deque.push_back(i);
    }

    // Test operator[]
    for (size_t i = 0; i < deque.size(); ++i) {
        EXPECT_EQ(deque[i], i);
    }

    // Test at() with valid indices
    for (size_t i = 0; i < deque.size(); ++i) {
        EXPECT_EQ(deque.at(i), i);
    }

    // Test at() with invalid index
    EXPECT_THROW(deque.at(deque.size()), std::out_of_range);
}

TEST(DequeTests, Iterators) {
    MockAllocator allocator;
    Deque<int> deque(&allocator);

    // Test empty container
    EXPECT_EQ(deque.begin(), deque.end());
    EXPECT_EQ(deque.cbegin(), deque.cend());

    // Fill with elements
    for (int i = 0; i < 10; ++i) {
        deque.push_back(i);
    }

    // Test iteration
    int expected = 0;
    for (const auto& value : deque) {
        EXPECT_EQ(value, expected++);
    }

    // Test const iteration
    const auto& const_deque = deque;
    expected = 0;
    for (const auto& value : const_deque) {
        EXPECT_EQ(value, expected++);
    }
}

TEST(DequeTests, Clear) {
    MockAllocator allocator;
    Deque<int> deque(&allocator);

    // Test clear on empty deque
    deque.clear();
    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(deque.size(), 0);

    // Fill with elements
    for (int i = 0; i < 10; ++i) {
        deque.push_back(i);
    }
    EXPECT_EQ(deque.size(), 10);

    // Clear and verify
    deque.clear();
    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(deque.size(), 0);

    // Verify we can still add elements after clear
    deque.push_back(42);
    EXPECT_EQ(deque.size(), 1);
    EXPECT_EQ(deque.front(), 42);
}

TEST(DequeTests, CopyConstructorAndAssignment) {
    pynovage::memory::tests::MockAllocator allocator;
    pynovage::memory::Deque<int> a(&allocator);
    for (int i = 0; i < 10; ++i) a.push_back(i);

    // Copy construct
    pynovage::memory::Deque<int> b = a;
    EXPECT_EQ(b.size(), a.size());
    for (size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(b[i], a[i]);
    }

    // Copy assign
    pynovage::memory::Deque<int> c(&allocator);
    c = a;
    EXPECT_EQ(c.size(), a.size());
    for (size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(c[i], a[i]);
    }
}

TEST(DequeTests, MoveConstructorAndAssignment) {
    pynovage::memory::tests::MockAllocator allocator;
    pynovage::memory::Deque<int> a(&allocator);
    for (int i = 0; i < 10; ++i) a.push_back(i);

    // Move construct
    pynovage::memory::Deque<int> b(std::move(a));
    EXPECT_EQ(b.size(), 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(b[i], i);
    }

    // Now move assign to c
    pynovage::memory::Deque<int> c(&allocator);
    c = std::move(b);
    EXPECT_EQ(c.size(), 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(c[i], i);
    }
}

TEST(DequeTests, Resize) {
    pynovage::memory::tests::MockAllocator allocator;
    pynovage::memory::Deque<int> d(&allocator);

    // Grow with default values
    d.resize(5);
    EXPECT_EQ(d.size(), 5);

    // Set known values and grow with value
    for (int i = 0; i < 5; ++i) d[i] = i;
    d.resize(8, 42);
    EXPECT_EQ(d.size(), 8);
    for (int i = 0; i < 5; ++i) EXPECT_EQ(d[i], i);
    for (int i = 5; i < 8; ++i) EXPECT_EQ(d[i], 42);

    // Shrink
    d.resize(3);
    EXPECT_EQ(d.size(), 3);
    for (int i = 0; i < 3; ++i) EXPECT_EQ(d[i], i);
}

} // namespace tests
} // namespace memory
} // namespace pynovage
