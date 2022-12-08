#include "AsyncTimer.h"
#include <thread>
#include <chrono>
uint64_t getTimeNs()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

AsyncTimer::AsyncTimer(uint32_t max_timers, uint64_t check_interval_ns)
    : max_timers_(max_timers),
      check_interval_ns_(check_interval_ns),
      qsize_(0),
      cur_ns_(0),
      running_(false)
{
    auto task_cmp = std::greater<AsyncTimerTask>();
    Container task_mem;
    task_mem.reserve(max_timers_);
    TaskQueue tmp(task_cmp, task_mem);
    tasks_queue_.swap(tmp);
}

AsyncTimer::~AsyncTimer()
{
    while (!tasks_queue_.empty())
    {
        tasks_queue_.top().run();
        tasks_queue_.pop();
    }
}

TimerInfo AsyncTimer::createNanoTimer(uint64_t ns, AsyncTimerTask::Cb cb, bool is_async)
{
    uint64_t cur_ns = 0;
    if (running_.load())
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (qsize_ == max_timers_)
            return {};
        if (cur_ns = getTimeNs(); cur_ns == 0)
            return {};
        ns += cur_ns;
        cur_ns_ = cur_ns;
        tasks_queue_.emplace(ns, cb, is_async);
        qsize_++;
        new_timer_event_.notify_one();
    }
    else
    {
        if (cur_ns = getTimeNs(); cur_ns == 0)
            return {};
        ns += cur_ns;
        cur_ns_ = cur_ns;
        tasks_queue_.emplace(ns, cb, is_async);
        qsize_++;
    }
    return {cur_ns, ns};
}

TimerInfo AsyncTimer::createMilliTimer(uint64_t ms, AsyncTimerTask::Cb cb, bool is_async)
{
    uint64_t ns = ms * 1'000'000;
    return createNanoTimer(ns, cb, is_async);
}

TimerInfo AsyncTimer::createSecTimer(uint32_t sec, AsyncTimerTask::Cb cb, bool is_async)
{
    uint64_t ns = static_cast<uint64_t>(sec) * 1'000'000'000;
    return createNanoTimer(ns, cb, is_async);
}

size_t AsyncTimer::checkTimers()
{
    uint64_t delay = 0;
    size_t count = 0;
    while (!tasks_queue_.empty() && tasks_queue_.top().ns <= cur_ns_)
    {
        count++;
        if (!tasks_queue_.top().is_async)
            tasks_queue_.top().cb();
        else
        {
            std::thread t(tasks_queue_.top().cb);
            t.detach();
        }
        cur_ns_ = getTimeNs();
        delay = cur_ns_ - tasks_queue_.top().ns;
        max_delay_ = std::max(max_delay_, delay);
        tasks_queue_.pop();
        qsize_--;
    }
    return count;
}

void AsyncTimer::checkTimersNow()
{
    if (running_.load())
    {
        // std::lock_guard<std::mutex> lock(mtx_);
        new_timer_event_.notify_one();
    }
    else
    {
        uint64_t cur_ns = 0;
        if (cur_ns = getTimeNs(); cur_ns != 0)
        {
            cur_ns_ = cur_ns;
            checkTimers();
        }
    }
}

void AsyncTimer::run(std::atomic_bool &terminate)
{
    uint64_t cur_ns = 0;
    uint64_t timeout = 0;
    std::unique_lock<std::mutex> lock(mtx_, std::defer_lock);
    running_.store(true);
    while (!terminate.load(std::memory_order_relaxed))
    {
        cur_ns = getTimeNs();
        if (cur_ns != 0)
        {
            lock.lock();
            cur_ns_ = cur_ns;
            if (!tasks_queue_.empty())
                timeout = std::min(check_interval_ns_, tasks_queue_.top().ns >= cur_ns_ ? tasks_queue_.top().ns - cur_ns_ : cur_ns_ - tasks_queue_.top().ns);
            else
                timeout = check_interval_ns_;
            new_timer_event_.wait_for(lock, std::chrono::nanoseconds(timeout));
            checkTimers();
            lock.unlock();
        }
    }
    running_.store(false);
}
