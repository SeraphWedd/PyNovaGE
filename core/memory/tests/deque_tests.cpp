#include "../include/deque.hpp"
#include <gtest/gtest.h>

namespace pynovage {
namespace memory {
namespace tests {

// Mock allocator for testing
class MockAllocator : public IAllocator {
public:
    void* allocateImpl(std::size_t size, std::size_t alignment) override {
        return ::operator new(size, std::align_val_t{alignment});
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

} // namespace tests
} // namespace memory
} // namespace pynovage