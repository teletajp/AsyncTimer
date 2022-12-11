#include "AsyncTimer.h"
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

uint64_t getTimeNs()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

AsyncTimer::AsyncTimer(uint32_t max_timers, uint64_t check_interval_ns)
    : max_timers_(max_timers),
      check_interval_ns_(check_interval_ns),
      qsize_(0),
      tasks_queue_(Comp(), Container(max_timers_)),
      cur_ns_(0),
      max_delay_(0),
      max_size_(0),
      running_(false),
      timer_info_id_(0)
{

    while (!tasks_queue_.empty())
        tasks_queue_.pop();
}

AsyncTimer::~AsyncTimer()
{
    while (!tasks_queue_.empty())
    {
        tasks_queue_.top().run();
        tasks_queue_.pop();
    }
}

TimerInfo AsyncTimer::addTimer_(uint64_t ns, AsyncTimerTask::Cb cb, bool is_async)
{
    uint64_t cur_ns = 0;
    if (qsize_ == max_timers_)
        return {};
    if (cur_ns = getTimeNs(); cur_ns == 0)
        return {};
    ns += cur_ns;
    cur_ns_ = cur_ns;
    tasks_queue_.emplace(ns, cb, ++timer_info_id_, is_async);
    qsize_++;
    return {timer_info_id_, cur_ns, ns};
}

TimerInfo AsyncTimer::createNanoTimer(uint64_t ns, AsyncTimerTask::Cb cb, bool is_async)
{
    TimerInfo ret;
    if (running_.load())
    {
        std::lock_guard lock(mtx_);
        ret = addTimer_(ns, cb, is_async);
        new_timer_event_.notify_one();
    }
    else
    {
        ret = addTimer_(ns, cb, is_async);
    }
    return ret;
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

bool AsyncTimer::delTimer_(uint64_t id, TaskQueue &nq)
{
    bool ret = false;
    while (!tasks_queue_.empty())
    {
        const auto &el = tasks_queue_.top();
        if (el.id != id)
        {
            nq.emplace(el);
        }
        else
        {
            ret = true;
        }
        tasks_queue_.pop();
    }
    tasks_queue_.swap(nq);
    return ret;
}

bool AsyncTimer::deleteTimer(uint64_t id)
{
    TaskQueue new_queue{Comp(), Container(max_timers_)};
    while (!new_queue.empty())
        new_queue.pop();

    if (running_.load())
    {
        std::lock_guard lock(mtx_);
        return delTimer_(id, new_queue);
    }
    return delTimer_(id, new_queue);
}

size_t AsyncTimer::checkTimers()
{
    uint64_t delay = 0;
    size_t count = 0;
    while (!tasks_queue_.empty() && tasks_queue_.top().ns <= cur_ns_)
    {
        count++;
        if (tasks_queue_.top().cb)
        {
            if (!tasks_queue_.top().is_async)
                tasks_queue_.top().cb();
            else
            {
                std::thread t(tasks_queue_.top().cb);
                t.detach();
            }
        }
        cur_ns_ = getTimeNs();
        delay = cur_ns_ - tasks_queue_.top().ns;
        max_delay_ = std::max(max_delay_, delay);
        max_size_ = std::max(max_size_, qsize_);
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
