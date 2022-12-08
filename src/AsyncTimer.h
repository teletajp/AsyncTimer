#pragma once
#include <cstdint>
#include <functional>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "Runnable.h"

/**
 * @brief Функция получения текущего времени в наносекундах
 *
 * @return uint64_t Кол-во наносекунд
 */
uint64_t getTimeNs();
/**
 * @brief Задание таймера
 *
 */
struct AsyncTimerTask
{
    using Cb = std::function<void()>;
    uint64_t ns = 0;       ///< Время сработки таймера в наносекундах
    bool is_async = false; ///< Асинхронное выполнение задания
    Cb cb;                 ///< Задание таймера

    AsyncTimerTask() = default;
    AsyncTimerTask(const AsyncTimerTask &o) = default;
    AsyncTimerTask(AsyncTimerTask &&o) = default;
    AsyncTimerTask operator=(const AsyncTimerTask &o)
    {
        if (&o != this)
        {
            ns = o.ns;
            cb = o.cb;
        }
        return *this;
    };
    AsyncTimerTask(uint64_t ns, Cb cb, bool is_async = false) : ns(ns), is_async(is_async), cb(cb){};
    ~AsyncTimerTask() = default;
    bool operator<(const AsyncTimerTask &o) const { return ns < o.ns; }
    bool operator>(const AsyncTimerTask &o) const { return ns > o.ns; }
    bool operator==(const AsyncTimerTask &o) const { return ns == o.ns; }
    /**
     * @brief Запуск задания таймера
     *
     */
    void run() const
    {
        if (cb)
            cb();
    }
};

struct TimerInfo
{
    union
    {
        uint64_t id = 0;
        uint64_t start_tm_ns;
    };
    uint64_t shedule_tm_ns = 0;
    TimerInfo() = default;
    TimerInfo(uint64_t start_tm_ns, uint64_t shedule_tm_ns) : start_tm_ns(start_tm_ns), shedule_tm_ns(shedule_tm_ns) {}
};
/**
 * @brief Асинхронный таймер
 *
 */
class AsyncTimer : public running::IRunnable
{
    using Container = std::vector<AsyncTimerTask>;
    using Comp = std::greater<AsyncTimerTask>;
    using TaskQueue = std::priority_queue<AsyncTimerTask, Container, Comp>;

private:
    const uint32_t max_timers_;
    const uint64_t check_interval_ns_;
    size_t qsize_;
    TaskQueue tasks_queue_;
    uint64_t cur_ns_;
    std::mutex mtx_;
    std::condition_variable new_timer_event_;
    uint64_t max_delay_ = 0;
    std::atomic_bool running_;

public:
    /**
     * @brief Конструктор с параметрами
     *
     * @param max_timers Максимальное количество таймеров
     * @param check_interval_ns Интервал проверки таймеров(наносек.)
     */
    AsyncTimer(uint32_t max_timers, uint64_t check_interval_ns);
    AsyncTimer() = delete;
    AsyncTimer(const AsyncTimer &) = delete;
    AsyncTimer(AsyncTimer &&) = delete;
    AsyncTimer &operator=(const AsyncTimer &) = delete;
    AsyncTimer &operator=(AsyncTimer &&) = delete;
    ~AsyncTimer();
    /**
     * @brief Создание таймера ожидающего ns наносекунд
     *
     * @param ns ожидание в наносекундах
     * @param cb функция выполняющаяся по истечении таймера
     * @param is_async асинхронное выполнение задания
     * @return uint64_t идентификатор таймера или 0 в случае ошибки
     */
    TimerInfo createNanoTimer(uint64_t ns, AsyncTimerTask::Cb cb, bool is_async = false);
    /**
     * @brief Создание таймера ожидающего ms милисекунд
     *
     * @param ms ожидание в милисекундах
     * @param cb функция выполняющаяся по истечении таймера
     * @param is_async асинхронное выполнение задания
     * @return uint64_t идентификатор таймера или 0 в случае ошибки
     */
    TimerInfo createMilliTimer(uint64_t ms, AsyncTimerTask::Cb cb, bool is_async = false);
    /**
     * @brief Создание таймера ожидающего sec секунд
     *
     * @param sec ожидание в секундах
     * @param cb функция выполняющаяся по истечении таймера
     * @param is_async асинхронное выполнение задания
     * @return uint64_t идентификатор таймера или 0 в случае ошибки
     */
    TimerInfo createSecTimer(uint32_t sec, AsyncTimerTask::Cb cb, bool is_async = false);
    // uint64_t deleteTimer(uint64_t id);
    /**
     * @brief Запуск цикла проверки таймеров
     *
     * @param terminate Флаг для остановки цикла проверки
     */
    void run(std::atomic_bool &terminate) override;
    /**
     * @brief Проверить таймеры сейчас (не дожидаясь истечения интервала таймера)
     *
     */
    void checkTimersNow();
    /**
     * @brief Получение максимальной задержки в наносекундах
     *
     * @return uint64_t
     * Не потокобезопасен
     */
    uint64_t maxDelay() const { return max_delay_; }

private:
    size_t checkTimers();
};