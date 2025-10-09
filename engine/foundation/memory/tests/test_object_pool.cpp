#include <gtest/gtest.h>
#include "memory/object_pool.h"

using namespace PyNovaGE;

struct TestObject {
    int value;
    float data;
    
    TestObject() : value(42), data(3.14f) {}
    TestObject(int v, float d) : value(v), data(d) {}
};

TEST(ObjectPoolTest, BasicAcquireAndRelease) {
    ObjectPool<TestObject> pool(5);
    
    EXPECT_EQ(pool.getAllocatedCount(), 0);
    EXPECT_EQ(pool.getFreeCount(), 5);
    
    TestObject* obj1 = pool.acquire();
    ASSERT_NE(obj1, nullptr);
    EXPECT_EQ(obj1->value, 42);
    EXPECT_EQ(obj1->data, 3.14f);
    
    EXPECT_EQ(pool.getAllocatedCount(), 1);
    EXPECT_EQ(pool.getFreeCount(), 4);
    
    TestObject* obj2 = pool.acquire(100, 2.71f);
    ASSERT_NE(obj2, nullptr);
    EXPECT_EQ(obj2->value, 100);
    EXPECT_EQ(obj2->data, 2.71f);
    
    pool.release(obj1);
    EXPECT_EQ(pool.getAllocatedCount(), 1);
    EXPECT_EQ(pool.getFreeCount(), 4);
    
    pool.release(obj2);
    EXPECT_EQ(pool.getAllocatedCount(), 0);
    EXPECT_EQ(pool.getFreeCount(), 5);
}

TEST(ObjectPoolTest, PoolExhaustion) {
    ObjectPool<TestObject> pool(2);
    
    TestObject* obj1 = pool.acquire();
    TestObject* obj2 = pool.acquire();
    TestObject* obj3 = pool.acquire(); // Should fail
    
    EXPECT_NE(obj1, nullptr);
    EXPECT_NE(obj2, nullptr);
    EXPECT_EQ(obj3, nullptr);
    
    EXPECT_EQ(pool.getAllocatedCount(), 2);
    EXPECT_EQ(pool.getFreeCount(), 0);
}

TEST(ObjectPoolTest, OwnsPointer) {
    ObjectPool<TestObject> pool(3);
    
    TestObject* obj = pool.acquire();
    EXPECT_TRUE(pool.ownsPointer(obj));
    
    TestObject* external_obj = new TestObject();
    EXPECT_FALSE(pool.ownsPointer(external_obj));
    
    pool.release(obj);
    delete external_obj;
}

TEST(ObjectPoolTest, Clear) {
    ObjectPool<TestObject> pool(5);
    
    pool.acquire();
    pool.acquire();
    pool.acquire();
    
    EXPECT_EQ(pool.getAllocatedCount(), 3);
    
    pool.clear();
    EXPECT_EQ(pool.getAllocatedCount(), 0);
    EXPECT_EQ(pool.getFreeCount(), 5);
}