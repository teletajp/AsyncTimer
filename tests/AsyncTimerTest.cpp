#include <gtest/gtest.h>
#include <gtest/internal/gtest-internal.h>
#include <AsyncTimer.h>
#include <chrono>
#include <thread>

#define TASK(N, T) []() { std::cout << "OnTimer" #N " time " #T << std::endl; }
using namespace std::chrono_literals;

class AsyncTimerTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(AsyncTimerTest, test1_000_000)
{
    const uint32_t max_tasks = 1'000'000;
    std::vector<uint64_t> task_ids;
    task_ids.reserve(max_tasks);
    AsyncTimer at(max_tasks, 1);
    {
        running::AutoThread thr(&at);
        std::this_thread::sleep_for(1s);
        for (uint32_t i = 0; i < 1'000'000; ++i)
        {
            at.createNanoTimer(1000 * i, []() { /*std::cout << "OnTimer" << i << " time " << 1000*i << std::endl;*/ });
        }
        std::this_thread::sleep_for(10s);
    }
    std::cout << "MAX_DELAY:" << at.maxDelay();
}

TEST_F(AsyncTimerTest, test100_000)
{
    const uint32_t max_tasks = 100'000;
    std::vector<uint64_t> task_ids;
    task_ids.reserve(max_tasks);
    AsyncTimer at(max_tasks, 1);
    {
        running::AutoThread thr(&at);
        std::this_thread::sleep_for(1s);
        for (uint32_t i = 0; i < max_tasks; ++i)
        {
            at.createNanoTimer(1000 * i, []() { /*std::cout << "OnTimer" << i << " time " << 1000*i << std::endl;*/ });
        }
        std::this_thread::sleep_for(10s);
    }
    std::cout << "MAX_DELAY:" << at.maxDelay();
}

TEST_F(AsyncTimerTest, test10_000)
{
    const uint32_t max_tasks = 10'000;
    std::vector<uint64_t> task_ids;
    task_ids.reserve(max_tasks);
    AsyncTimer at(max_tasks, 1);
    {
        running::AutoThread thr(&at);
        std::this_thread::sleep_for(1s);
        for (uint32_t i = 0; i < max_tasks; ++i)
        {
            at.createNanoTimer(1000 * i, []() { /*std::cout << "OnTimer" << i << " time " << 1000*i << std::endl;*/ });
        }
        std::this_thread::sleep_for(10s);
    }
    std::cout << "MAX_DELAY:" << at.maxDelay();
}

TEST_F(AsyncTimerTest, test)
{
    const uint32_t max_tasks = 10'000;
    std::vector<uint64_t> task_ids;
    task_ids.reserve(max_tasks);
    AsyncTimer at(max_tasks, 1);
    {
        running::AutoThread thr(&at);
        std::this_thread::sleep_for(1s);
        task_ids.push_back(at.createSecTimer(1, []()
                                             { std::cout << "time " << getTimeNs() << std::endl; }));
        task_ids.push_back(at.createSecTimer(10, []()
                                             { std::cout << "time " << getTimeNs() << std::endl; }));
        task_ids.push_back(at.createSecTimer(13, []()
                                             { std::cout << "time " << getTimeNs() << std::endl; }));

        std::this_thread::sleep_for(20s);
        for (auto id : task_ids)
            std::cout << "id " << id << std::endl;
    }
    std::cout << "MAX_DELAY:" << at.maxDelay();
}

TEST_F(AsyncTimerTest, test_async)
{
    const uint32_t max_tasks = 10'000;
    std::vector<uint64_t> task_ids;
    task_ids.reserve(max_tasks);
    AsyncTimer at(max_tasks, 1);
    {
        running::AutoThread thr(&at);
        std::this_thread::sleep_for(1s);
        task_ids.push_back(at.createSecTimer(
            1, []()
            { std::cout << "time " << getTimeNs() << std::endl; },
            true));
        task_ids.push_back(at.createSecTimer(
            10, []()
            { std::cout << "time " << getTimeNs() << std::endl; },
            true));
        task_ids.push_back(at.createSecTimer(
            13, []()
            { std::cout << "time " << getTimeNs() << std::endl; },
            true));

        std::this_thread::sleep_for(20s);
    }
    for (auto id : task_ids)
        std::cout << "id " << id << std::endl;
    std::cout << "MAX_DELAY:" << at.maxDelay();
}

TEST_F(AsyncTimerTest, test_max_tasks)
{
    const uint32_t max_tasks = 2;
    AsyncTimer at(max_tasks, 1);
    {
        running::AutoThread thr(&at);
        std::this_thread::sleep_for(1s);
        ASSERT_TRUE(at.createSecTimer(10, TASK(1, 10)));
        ASSERT_TRUE(at.createSecTimer(20, TASK(2, 20)));
        ASSERT_FALSE(at.createSecTimer(15, TASK(3, 15)));

        std::this_thread::sleep_for(25s);
    }
    std::cout << "MAX_DELAY:" << at.maxDelay();
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    // getchar();
    return ret;
}