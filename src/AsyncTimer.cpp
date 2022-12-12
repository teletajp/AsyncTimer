#include "AsyncTimer.h"
#include "AllocatorOnArray.h"
#include <thread>
#include <chrono>
#include <mutex>
#include <list>
#include <algorithm>

using namespace std::chrono_literals;

uint64_t getTimeNs()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

/**
 * @brief Задание таймера
 *
 */
struct alignas(64) AsyncTimerTask
{
    uint64_t ns = 0;        ///< Время сработки таймера в наносекундах
    bool is_async = false;  ///< Асинхронное выполнение задания
    TaskCbPtr cb = nullptr; ///< Задание таймера
    uint64_t id = 0;        ///< id таймера

    AsyncTimerTask() = default;
    AsyncTimerTask(const AsyncTimerTask &o) = default;
    AsyncTimerTask(AsyncTimerTask &&o) = default;
    AsyncTimerTask &operator=(const AsyncTimerTask &o)
    {
        if (&o != this)
        {
            ns = o.ns;
            is_async = o.is_async;
            cb = o.cb;
            id = o.id;
        }
        return *this;
    };
    AsyncTimerTask(uint64_t ns, TaskCbPtr &cb, uint64_t id, bool is_async = false) : ns(ns), id(id), is_async(is_async), cb(cb){};
    ~AsyncTimerTask();
    bool operator<(const AsyncTimerTask &o) const { return ns < o.ns; }
    bool operator>(const AsyncTimerTask &o) const { return ns > o.ns; }
    bool operator==(const AsyncTimerTask &o) const { return ns == o.ns; }
    void run()
    {
        if (cb)
            cb->run();
    }
};

AsyncTimerTask::~AsyncTimerTask()
{
    if (cb)
        cb.reset();
}

static constexpr size_t MAX_TASKS = 1'000'000;
using alloc_type = AllocatorOnArray<AsyncTimerTask, MAX_TASKS>;
class AsyncTimer::Impl
{
public:
    using TimerList = std::list<AsyncTimerTask, alloc_type>;
    const uint32_t max_timers;
    const uint64_t check_interval_ns;
    std::atomic_bool running;
    TimerList active_timers;
    std::mutex mtx;
    uint64_t timer_id;
    uint32_t max_size;
    uint64_t max_delay;
    uint64_t cur_tm;
    Impl(uint32_t max_timers, uint64_t check_interval_ns);
    ~Impl();
    TimerInfo addTimer(uint64_t ns, TaskCbPtr &cb, bool is_async);
    TimerInfo createTimer(uint64_t ns, TaskCbPtr &cb, bool is_async);
    bool delTimer(uint64_t id);
    bool deleteTimer(uint64_t id);
    void run(std::atomic_bool &terminate);
    size_t checkTimers();
    size_t checkNow();
};

AsyncTimer::Impl::Impl(uint32_t max_timers, uint64_t check_interval_ns)
    : max_timers(max_timers > MAX_TASKS ? MAX_TASKS : max_timers),
      check_interval_ns(check_interval_ns), running(false),
      active_timers(),
      timer_id(0), max_size(0), max_delay(0), cur_tm(getTimeNs())
{
}

AsyncTimer::Impl::~Impl()
{
    for (auto &t : active_timers)
    {
        t.cb->run();
    }
}

TimerInfo AsyncTimer::Impl::addTimer(uint64_t ns, TaskCbPtr &cb, bool is_async)
{
    if (active_timers.size() == max_timers)
        return {};
    cur_tm = getTimeNs();
    active_timers.emplace_back(ns + cur_tm, cb, ++timer_id, is_async);
    max_size = std::max(static_cast<uint32_t>(active_timers.size()), max_size);
    return {timer_id, cur_tm, cur_tm + ns};
}
TimerInfo AsyncTimer::Impl::createTimer(uint64_t ns, TaskCbPtr &cb, bool is_async)
{
    if (running.load(std::memory_order::memory_order_acquire))
    {
        std::lock_guard lock(mtx);
        return addTimer(ns, cb, is_async);
    }
    return addTimer(ns, cb, is_async);
}

bool AsyncTimer::Impl::delTimer(uint64_t id)
{
    auto it = std::find_if(active_timers.begin(), active_timers.end(), [id](const auto &task)
                           { return task.id == id; });
    if (it != active_timers.end())
    {
        active_timers.erase(it);
        return true;
    }
    return false;
}

bool AsyncTimer::Impl::deleteTimer(uint64_t id)
{
    if (running.load(std::memory_order::memory_order_acquire))
    {
        std::lock_guard lock(mtx);
        return delTimer(id);
    }
    return delTimer(id);
}

size_t AsyncTimer::Impl::checkTimers()
{
    size_t count = 0;
    for (auto it = active_timers.begin(); it != active_timers.end();)
    {
        if (it->ns <= cur_tm)
        {
            max_delay = std::max(static_cast<uint64_t>(cur_tm - it->ns), max_delay);
            if (!it->is_async)
            {
                it->cb->run();
            }
            else
            {
                std::thread th(&AsyncTimerTaskCb::run, it->cb);
                th.detach();
            }
            it = active_timers.erase(it);
            count++;
        }
        else
        {
            it++;
        }
    }
    return count;
}

size_t AsyncTimer::Impl::checkNow()
{
    if (running.load())
    {
        std::lock_guard lock(mtx);
        cur_tm = getTimeNs();
        return checkTimers();
    }
    cur_tm = getTimeNs();
    return checkTimers();
}

void AsyncTimer::Impl::run(std::atomic_bool &terminate)
{
    int64_t elapsed_tm = 0;
    running.store(true, std::memory_order::memory_order_release);
    while (!terminate.load())
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(check_interval_ns));
        std::lock_guard lock(mtx);
        cur_tm = getTimeNs();
        checkTimers();
    }
    running.store(false, std::memory_order::memory_order_release);
}

AsyncTimer::AsyncTimer(uint32_t max_timers, uint64_t check_interval_ns)
    : pimpl_(std::make_unique<Impl>(max_timers, check_interval_ns))
{
}

AsyncTimer::~AsyncTimer()
{
}

TimerInfo AsyncTimer::createNanoTimer(uint64_t ns, TaskCbPtr &cb, bool is_async)
{
    if (cb)
        return pimpl_->createTimer(ns, cb, is_async);
    return {};
}

TimerInfo AsyncTimer::createMilliTimer(uint64_t ms, TaskCbPtr &cb, bool is_async)
{
    uint64_t ns = ms * 1'000'000;
    return createNanoTimer(ns, cb, is_async);
}

TimerInfo AsyncTimer::createSecTimer(uint32_t sec, TaskCbPtr &cb, bool is_async)
{
    uint64_t ns = static_cast<uint64_t>(sec) * 1'000'000'000;
    return createNanoTimer(ns, cb, is_async);
}

bool AsyncTimer::deleteTimer(uint64_t id)
{
    return pimpl_->deleteTimer(id);
}

size_t AsyncTimer::checkTimersNow()
{
    return pimpl_->checkNow();
}

void AsyncTimer::run(std::atomic_bool &terminate)
{
    pimpl_->run(terminate);
}

uint64_t AsyncTimer::maxDelay()
{
    return pimpl_->max_delay;
}

uint64_t AsyncTimer::maxSize()
{
    return pimpl_->max_size;
}