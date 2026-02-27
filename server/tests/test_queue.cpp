#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <atomic>
#include <chrono>
#include "../customQueue.h"

// --- Basic Operations ---

TEST(CustomQueueTest, PushAndPop) {
    customQueue<int> q(5);

    q.push(10);
    q.push(20);

    EXPECT_EQ(q.pop(), 10);
    EXPECT_EQ(q.pop(), 20);
}

TEST(CustomQueueTest, QueueSizeAndCapacity) {
    customQueue<int> q(2);

    q.push(1);
    q.push(2);
    EXPECT_EQ(q.size(), 2);
    EXPECT_EQ(q.getCapacity(), 2);
}

TEST(CustomQueueTest, FIFOOrdering) {
    customQueue<int> q;
    const int N = 100;

    for (int i = 0; i < N; i++) {
        q.push(i);
    }

    for (int i = 0; i < N; i++) {
        EXPECT_EQ(q.pop(), i);
    }
}

TEST(CustomQueueTest, UnlimitedCapacity) {
    customQueue<int> q; // default capacity = 0 (unlimited)
    EXPECT_EQ(q.getCapacity(), 0);

    for (int i = 0; i < 1000; i++) {
        q.push(i);
    }
    EXPECT_EQ(q.size(), 1000);
}

TEST(CustomQueueTest, SetCapacity) {
    customQueue<int> q;
    EXPECT_EQ(q.getCapacity(), 0);

    q.setCapacity(10);
    EXPECT_EQ(q.getCapacity(), 10);

    q.setCapacity(5);
    EXPECT_EQ(q.getCapacity(), 5);
}

TEST(CustomQueueTest, SizeAfterPops) {
    customQueue<int> q;

    q.push(1);
    q.push(2);
    q.push(3);
    EXPECT_EQ(q.size(), 3);

    q.pop();
    EXPECT_EQ(q.size(), 2);

    q.pop();
    q.pop();
    EXPECT_EQ(q.size(), 0);
}

TEST(CustomQueueTest, StringType) {
    customQueue<std::string> q;

    q.push("hello");
    q.push("world");

    EXPECT_EQ(q.pop(), "hello");
    EXPECT_EQ(q.pop(), "world");
}

// --- Concurrency Tests ---

TEST(CustomQueueTest, ConcurrentProducerConsumer) {
    customQueue<int> q(100);
    const int N = 10000;
    std::vector<int> consumed;
    std::mutex consumed_mutex;

    std::thread producer([&]() {
        for (int i = 0; i < N; i++) {
            q.push(i);
        }
    });

    std::thread consumer([&]() {
        for (int i = 0; i < N; i++) {
            int val = q.pop();
            std::lock_guard<std::mutex> lock(consumed_mutex);
            consumed.push_back(val);
        }
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(consumed.size(), N);

    std::sort(consumed.begin(), consumed.end());
    for (int i = 0; i < N; i++) {
        EXPECT_EQ(consumed[i], i);
    }
}

TEST(CustomQueueTest, MultipleProducersMultipleConsumers) {
    customQueue<int> q(50);
    const int NUM_PRODUCERS = 4;
    const int NUM_CONSUMERS = 4;
    const int ITEMS_PER_PRODUCER = 2500;
    const int TOTAL = NUM_PRODUCERS * ITEMS_PER_PRODUCER;

    std::atomic<int> totalConsumed{0};
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    for (int p = 0; p < NUM_PRODUCERS; p++) {
        producers.emplace_back([&, p]() {
            for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
                q.push(p * ITEMS_PER_PRODUCER + i);
            }
        });
    }

    for (int c = 0; c < NUM_CONSUMERS; c++) {
        consumers.emplace_back([&]() {
            while (totalConsumed.fetch_add(1) < TOTAL) {
                q.pop();
            }
        });
    }

    for (auto &t : producers) t.join();
    for (auto &t : consumers) t.join();

    EXPECT_EQ(q.size(), 0);
}

TEST(CustomQueueTest, CapacityBlocksProducer) {
    customQueue<int> q(2);

    q.push(1);
    q.push(2);

    std::atomic<bool> pushCompleted{false};

    std::thread producer([&]() {
        q.push(3); // should block since queue is full
        pushCompleted.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(pushCompleted.load());

    q.pop(); // free space

    producer.join();
    EXPECT_TRUE(pushCompleted.load());
    EXPECT_EQ(q.pop(), 2);
    EXPECT_EQ(q.pop(), 3);
}

TEST(CustomQueueTest, PopBlocksOnEmpty) {
    customQueue<int> q;

    std::atomic<bool> popCompleted{false};
    int poppedValue = -1;

    std::thread consumer([&]() {
        poppedValue = q.pop(); // should block on empty
        popCompleted.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(popCompleted.load());

    q.push(42); // unblock

    consumer.join();
    EXPECT_TRUE(popCompleted.load());
    EXPECT_EQ(poppedValue, 42);
}

// --- Sentinel Value Pattern (used in XDBC pipeline) ---

TEST(CustomQueueTest, SentinelTermination) {
    customQueue<int> q;
    const int NUM_ITEMS = 100;
    std::vector<int> results;

    std::thread producer([&]() {
        for (int i = 0; i < NUM_ITEMS; i++) {
            q.push(i);
        }
        q.push(-1); // sentinel
    });

    std::thread consumer([&]() {
        while (true) {
            int val = q.pop();
            if (val == -1) break;
            results.push_back(val);
        }
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(results.size(), NUM_ITEMS);
    for (int i = 0; i < NUM_ITEMS; i++) {
        EXPECT_EQ(results[i], i);
    }
}

TEST(CustomQueueTest, MultipleSentinels) {
    customQueue<int> q;
    const int NUM_CONSUMERS = 4;
    std::atomic<int> finishedConsumers{0};

    std::thread producer([&]() {
        for (int i = 0; i < 100; i++) {
            q.push(i);
        }
        for (int i = 0; i < NUM_CONSUMERS; i++) {
            q.push(-1);
        }
    });

    std::vector<std::thread> consumers;
    for (int c = 0; c < NUM_CONSUMERS; c++) {
        consumers.emplace_back([&]() {
            while (true) {
                int val = q.pop();
                if (val == -1) break;
            }
            finishedConsumers.fetch_add(1);
        });
    }

    producer.join();
    for (auto &t : consumers) t.join();

    EXPECT_EQ(finishedConsumers.load(), NUM_CONSUMERS);
}
