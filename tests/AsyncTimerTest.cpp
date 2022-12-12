#include <gtest/gtest.h>
#include <gtest/internal/gtest-internal.h>
#include <AsyncTimer.h>
#include <chrono>
#include <thread>
#include <random>

#define TASK(N, T) []() { std::cout << "OnTimer" #N " time " #T << std::endl; }
using namespace std::chrono_literals;

class AsyncTimerTest : public ::testing::Test
{
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

struct TestTask : public AsyncTimerTaskCb
{
    std::function<void()> fn;
    TestTask(std::function<void()> fn) : fn(fn) {}
    void run() override
    {
        if (fn)
            fn();
    }
};

TEST_F(AsyncTimerTest, test1_000_000)
{
    const uint32_t max_tasks = 1'000'000;
    std::vector<TimerInfo> task_ids;
    std::vector<uint64_t> task_stop_times(max_tasks);
    task_ids.reserve(max_tasks);
    AsyncTimer at(max_tasks, 1'000'000);
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<unsigned long long> distrib(1'000, 10'000'000'000);
        running::AutoThread thr(&at);
        std::this_thread::sleep_for(1s);
        srand(time(NULL));
        for (uint32_t i = 0; i < max_tasks; ++i)
        {
            std::shared_ptr<AsyncTimerTaskCb> task_ptr = std::make_shared<TestTask>([i, &task_stop_times]()
                                                                                    { task_stop_times[i] = getTimeNs(); });
            task_ids.push_back(at.createNanoTimer(distrib(gen), task_ptr));
            std::this_thread::sleep_for(1000ns);
        }
        std::this_thread::sleep_for(10s);
    }

    /* Раскоментировать для генерации отчета в csv: ./x64-osx/bin/tests/async_timer_test --gtest_filter=AsyncTimerTest.test1_000_000 > out.csv*/
    // std::cout << "start_tm_ns;shedule_tm_ns;stop_tm\n";
    uint64_t max_delay = 0;
    for (uint32_t i = 0; i < max_tasks; ++i)
    {
        max_delay = std::max(max_delay, task_stop_times[i] - task_ids[i].shedule_tm_ns);
    }
    std::cout << "MAX_DELAY:" << max_delay << " MAX_SIZE:" << at.maxSize() << std::endl;
}

TEST_F(AsyncTimerTest, test100_000)
{
    const uint32_t max_tasks = 100'000;
    std::vector<TimerInfo> task_ids;
    std::vector<uint64_t> task_stop_times(max_tasks);
    task_ids.reserve(max_tasks);
    AsyncTimer at(max_tasks, 1'000'000);
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<unsigned long long> distrib(1'000, 9'000'000'000);
        running::AutoThread thr(&at);
        std::this_thread::sleep_for(3s);
        srand(time(NULL));
        for (uint32_t i = 0; i < max_tasks; ++i)
        {
            std::shared_ptr<AsyncTimerTaskCb> task_ptr = std::make_shared<TestTask>([i, &task_stop_times]()
                                                                                    { task_stop_times[i] = getTimeNs(); });
            task_ids.push_back(at.createNanoTimer(distrib(gen), task_ptr));
            std::this_thread::sleep_for(1000ns);
        }
        std::this_thread::sleep_for(10s);
    }
    std::cout << "MAX_DELAY:" << at.maxDelay() << " MAX_SIZE:" << at.maxSize() << std::endl;
    /* Раскоментировать для генерации отчета в csv: ./x64-osx/bin/tests/async_timer_test --gtest_filter=AsyncTimerTest.test100_000 > out.csv*/
    uint64_t max_delay = 0;
    for (uint32_t i = 0; i < max_tasks; ++i)
    {
        max_delay = std::max(max_delay, task_stop_times[i] - task_ids[i].shedule_tm_ns);
    }
    std::cout << "MAX_DELAY:" << max_delay << " MAX_SIZE:" << at.maxSize() << std::endl;
}

TEST_F(AsyncTimerTest, test10_000)
{
    const uint32_t max_tasks = 10'000;
    std::vector<TimerInfo> task_ids;
    std::vector<uint64_t> task_stop_times(max_tasks);
    task_ids.reserve(max_tasks);
    AsyncTimer at(max_tasks, 1'000'000);
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<unsigned long long> distrib(1'000, 9'000'000'000);
        running::AutoThread thr(&at);
        std::this_thread::sleep_for(1s);
        srand(time(NULL));
        for (uint32_t i = 0; i < max_tasks; ++i)
        {
            std::shared_ptr<AsyncTimerTaskCb> task_ptr = std::make_shared<TestTask>([i, &task_stop_times]()
                                                                                    { task_stop_times[i] = getTimeNs(); });
            task_ids.push_back(at.createNanoTimer(distrib(gen), task_ptr));
            std::this_thread::sleep_for(1000ns);
        }
        std::this_thread::sleep_for(10s);
    }
    std::cout << "MAX_DELAY:" << at.maxDelay() << " MAX_SIZE:" << at.maxSize() << std::endl;
    uint64_t max_delay = 0;
    for (uint32_t i = 0; i < max_tasks; ++i)
    {
        max_delay = std::max(max_delay, task_stop_times[i] - task_ids[i].shedule_tm_ns);
    }
    std::cout << "MAX_DELAY:" << max_delay << " MAX_SIZE:" << at.maxSize() << std::endl;
}

TEST_F(AsyncTimerTest, test)
{
    const uint32_t max_tasks = 10'000;
    std::vector<TimerInfo> task_ids;
    task_ids.reserve(max_tasks);
    AsyncTimer at(max_tasks, 1'000'000);
    {
        running::AutoThread thr(&at);
        std::this_thread::sleep_for(1s);
        std::shared_ptr<AsyncTimerTaskCb> task_ptr1 = std::make_shared<TestTask>([]()
                                                                                 { std::cout << "timer 1 1s stop at " << getTimeNs() << "ns" << std::endl; });
        std::shared_ptr<AsyncTimerTaskCb> task_ptr2 = std::make_shared<TestTask>([]()
                                                                                 { std::cout << "timer 2 10s stop at " << getTimeNs() << "ns" << std::endl; });
        std::shared_ptr<AsyncTimerTaskCb> task_ptr3 = std::make_shared<TestTask>([]()
                                                                                 { std::cout << "timer 3 13s stop at " << getTimeNs() << "ns" << std::endl; });
        task_ids.push_back(at.createSecTimer(1, task_ptr1));
        task_ids.push_back(at.createSecTimer(10, task_ptr2));
        task_ids.push_back(at.createSecTimer(13, task_ptr3));

        std::this_thread::sleep_for(20s);
        uint32_t i = 1;
        for (auto t : task_ids)
            std::cout << "timer " << i++ << " start time " << t.start_tm_ns << " ns "
                      << "shedule time " << t.shedule_tm_ns << std::endl;
    }
    std::cout << "MAX_DELAY:" << at.maxDelay() << std::endl;
}

TEST_F(AsyncTimerTest, test_async)
{
    const uint32_t max_tasks = 10'000;
    std::vector<TimerInfo> task_ids;
    task_ids.reserve(max_tasks);
    AsyncTimer at(max_tasks, 1'000'000);
    {
        running::AutoThread thr(&at);
        std::this_thread::sleep_for(1s);
        std::shared_ptr<AsyncTimerTaskCb> task_ptr1 = std::make_shared<TestTask>([]()
                                                                                 { std::cout << "timer 1 1s stop at " << getTimeNs() << "ns" << std::endl; });
        std::shared_ptr<AsyncTimerTaskCb> task_ptr2 = std::make_shared<TestTask>([]()
                                                                                 { std::cout << "timer 2 10s stop at " << getTimeNs() << "ns" << std::endl; });
        std::shared_ptr<AsyncTimerTaskCb> task_ptr3 = std::make_shared<TestTask>([]()
                                                                                 { std::cout << "timer 3 13s stop at " << getTimeNs() << "ns" << std::endl; });
        task_ids.push_back(at.createSecTimer(1, task_ptr1, true));
        task_ids.push_back(at.createSecTimer(10, task_ptr2, true));
        task_ids.push_back(at.createSecTimer(13, task_ptr3, true));

        std::this_thread::sleep_for(20s);
    }
    uint32_t i = 1;
    for (auto t : task_ids)
        std::cout << "timer " << i++ << " start time " << t.start_tm_ns << " ns "
                  << "shedule time " << t.shedule_tm_ns << std::endl;
    std::cout << "MAX_DELAY:" << at.maxDelay() << std::endl;
}

TEST_F(AsyncTimerTest, test_max_tasks)
{
    const uint32_t max_tasks = 2;
    AsyncTimer at(max_tasks, 1'000'000);
    {
        running::AutoThread thr(&at);
        std::this_thread::sleep_for(1s);
        std::shared_ptr<AsyncTimerTaskCb> task_ptr1 = std::make_shared<TestTask>(TASK(1, 10));
        std::shared_ptr<AsyncTimerTaskCb> task_ptr2 = std::make_shared<TestTask>(TASK(2, 20));
        std::shared_ptr<AsyncTimerTaskCb> task_ptr3 = std::make_shared<TestTask>(TASK(3, 15));
        std::shared_ptr<AsyncTimerTaskCb> task_ptr4 = std::make_shared<TestTask>(TASK(3, 5));
        ASSERT_TRUE(at.createSecTimer(10, task_ptr1).id);
        ASSERT_TRUE(at.createSecTimer(20, task_ptr2).id);
        ASSERT_FALSE(at.createSecTimer(15, task_ptr3).id);
        std::this_thread::sleep_for(11s);
        ASSERT_TRUE(at.createSecTimer(5, task_ptr4).id);
        std::this_thread::sleep_for(10s);
    }
    std::cout << "MAX_DELAY:" << at.maxDelay() << std::endl;
}

TEST_F(AsyncTimerTest, test_no_running)
{
    const uint32_t max_tasks = 10;
    AsyncTimer at(max_tasks, 1'000'000);
    for (uint32_t i = 1; i <= max_tasks; ++i)
    {
        std::shared_ptr<AsyncTimerTaskCb> task_ptr1 = std::make_shared<TestTask>([i]()
                                                                                 { std::cout << "OnTimer" << i << std::endl; });
        ASSERT_TRUE(at.createSecTimer(i, task_ptr1)
                        .id);
    }
    at.checkTimersNow();
    std::cout << "WAIT 3s:" << std::endl;
    std::this_thread::sleep_for(3s);
    at.checkTimersNow();
    std::cout << "WAIT 3s:" << std::endl;
    std::this_thread::sleep_for(3s);
    at.checkTimersNow();
    std::cout << "WAIT 3s:" << std::endl;
    std::this_thread::sleep_for(3s);
    at.checkTimersNow();
    std::cout << "WAIT 3s:" << std::endl;
    std::this_thread::sleep_for(3s);
    at.checkTimersNow();
    std::cout << "MAX_DELAY:" << at.maxDelay() << std::endl;
}

TEST_F(AsyncTimerTest, test_no_running_one_task)
{
    const uint32_t max_tasks = 10;
    AsyncTimer at(max_tasks, 1'000'000);
    std::shared_ptr<AsyncTimerTaskCb> task_ptr1 = std::make_shared<TestTask>(TASK(1, 1));
    ASSERT_TRUE(at.createNanoTimer(1, task_ptr1).id);
    at.checkTimersNow();
    std::cout << "MAX_DELAY:" << at.maxDelay() << std::endl;
}

TEST_F(AsyncTimerTest, test_del_task)
{
    const uint32_t max_tasks = 10;
    AsyncTimer at(max_tasks, 1'000'000);
    std::shared_ptr<AsyncTimerTaskCb> task_ptr1 = std::make_shared<TestTask>(TASK(1, 1));
    std::shared_ptr<AsyncTimerTaskCb> task_ptr2 = std::make_shared<TestTask>(TASK(2, 10));
    std::shared_ptr<AsyncTimerTaskCb> task_ptr3 = std::make_shared<TestTask>(TASK(3, 100));
    auto it1 = at.createNanoTimer(1, task_ptr1);
    auto it2 = at.createNanoTimer(10, task_ptr2);
    auto it3 = at.createNanoTimer(100, task_ptr3);
    at.deleteTimer(it2.id);
    std::this_thread::sleep_for(1s);
    at.checkTimersNow();
    std::cout << "MAX_DELAY:" << at.maxDelay() << std::endl;
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    // getchar();
    return ret;
}
