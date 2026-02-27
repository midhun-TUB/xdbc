#include <gtest/gtest.h>
#include "../customQueue.h"

TEST(CustomQueueTest, PushAndPop) {
    customQueue<int> q(5);
    
    q.push(10);
    q.push(20);
    
    EXPECT_EQ(q.pop(), 10);
    EXPECT_EQ(q.pop(), 20);
}

// Note: Testing full capacity is tricky due to the thread-blocking `wait` inside push.
// We'll just test size bounds and basic interactions here.
TEST(CustomQueueTest, QueueSizeAndCapacity) {
    customQueue<int> q(2);
    
    q.push(1);
    q.push(2);
    EXPECT_EQ(q.size(), 2);
    EXPECT_EQ(q.getCapacity(), 2);
}
