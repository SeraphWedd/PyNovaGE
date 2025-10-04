#include "../include/deque.hpp"
#include <gtest/gtest.h>

namespace pynovage {
namespace memory {
namespace tests {

TEST(DequeTests, DefaultConstructor) {
    Deque<int> deque;
    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(deque.size(), 0);
}

TEST(DequeTests, AllocatorConstructor) {
    // We'll need to mock an allocator later, for now just test with nullptr
    Deque<int> deque(nullptr);
    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(deque.size(), 0);
}

} // namespace tests
} // namespace memory
} // namespace pynovage