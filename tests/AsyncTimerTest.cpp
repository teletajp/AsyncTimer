#include <gtest/gtest.h>
#include <gtest/internal/gtest-internal.h>
#include <AsyncTimer.h>

class AsyncTimerTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

#define TASK(N, T) []() { std::cout << "OnTimer" #N " time " #T << std::endl; }

TEST_F(AsyncTimerTest, test1_000_000)
{
    const uint32_t max_tasks = 1'000'000;
    std::vector<uint64_t> task_ids;
    task_ids.reserve(max_tasks);
    AsyncTimer at(max_tasks, 1);
    running::AutoThread thr(&at);
    sleep(1);
    for (uint32_t i = 0; i < 1'000'000; ++i)
    {
        at.createNanoTimer(1000 * i, [=]() { /*std::cout << "OnTimer" << i << " time " << 1000*i << std::endl;*/ });
    }
    sleep(10);
    ASSERT_TRUE(true);
}

TEST_F(AsyncTimerTest, test100_000)
{
    const uint32_t max_tasks = 100'000;
    std::vector<uint64_t> task_ids;
    task_ids.reserve(max_tasks);
    AsyncTimer at(max_tasks, 1);
    running::AutoThread thr(&at);
    sleep(1);
    for (uint32_t i = 0; i < max_tasks; ++i)
    {
        at.createNanoTimer(1000 * i, [=]() { /*std::cout << "OnTimer" << i << " time " << 1000*i << std::endl;*/ });
    }
    sleep(10);
    ASSERT_TRUE(true);
}

TEST_F(AsyncTimerTest, test10_000)
{
    const uint32_t max_tasks = 10'000;
    std::vector<uint64_t> task_ids;
    task_ids.reserve(max_tasks);
    AsyncTimer at(max_tasks, 1);
    running::AutoThread thr(&at);
    sleep(1);
    for (uint32_t i = 0; i < max_tasks; ++i)
    {
        at.createNanoTimer(1000 * i, [=]() { /*std::cout << "OnTimer" << i << " time " << 1000*i << std::endl;*/ });
    }
    sleep(10);
    ASSERT_TRUE(true);
}

TEST_F(AsyncTimerTest, test)
{
    const uint32_t max_tasks = 10'000;
    std::vector<uint64_t> task_ids;
    task_ids.reserve(max_tasks);
    AsyncTimer at(max_tasks, 1);
    running::AutoThread thr(&at);
    sleep(1);
    task_ids.push_back(at.createSecTimer(1, [=]()
                                         { std::cout << "time " << getTimeNs() << std::endl; }));
    task_ids.push_back(at.createSecTimer(10, [=]()
                                         { std::cout << "time " << getTimeNs() << std::endl; }));
    task_ids.push_back(at.createSecTimer(13, [=]()
                                         { std::cout << "time " << getTimeNs() << std::endl; }));

    sleep(20);
    for (auto id : task_ids)
        std::cout << "id " << id << std::endl;
    ASSERT_TRUE(true);
}

TEST_F(AsyncTimerTest, test_async)
{
    const uint32_t max_tasks = 10'000;
    std::vector<uint64_t> task_ids;
    task_ids.reserve(max_tasks);
    AsyncTimer at(max_tasks, 1);
    running::AutoThread thr(&at);
    sleep(1);
    task_ids.push_back(at.createSecTimer(
        1, [=]()
        { std::cout << "time " << getTimeNs() << std::endl; },
        true));
    task_ids.push_back(at.createSecTimer(
        10, [=]()
        { std::cout << "time " << getTimeNs() << std::endl; },
        true));
    task_ids.push_back(at.createSecTimer(
        13, [=]()
        { std::cout << "time " << getTimeNs() << std::endl; },
        true));

    sleep(20);
    for (auto id : task_ids)
        std::cout << "id " << id << std::endl;
    ASSERT_TRUE(true);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    // getchar();
    return ret;
}