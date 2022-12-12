#pragma once
#include <cstdint>
#include <memory>
#include <functional>
#include "Runnable.h"

/**
 * @brief Функция получения текущего времени в наносекундах
 *
 * @return uint64_t Кол-во наносекунд
 */
uint64_t getTimeNs();

struct AsyncTimerTaskCb
{
    virtual void run() = 0;
    virtual ~AsyncTimerTaskCb() = default;
};
using TaskCbPtr = std::shared_ptr<AsyncTimerTaskCb>;
struct TimerInfo
{
    uint64_t id = 0;
    uint64_t start_tm_ns = 0;
    uint64_t shedule_tm_ns = 0;
    TimerInfo() = default;
    TimerInfo(uint64_t id, uint64_t start_tm_ns, uint64_t shedule_tm_ns) : id(id), start_tm_ns(start_tm_ns), shedule_tm_ns(shedule_tm_ns) { id++; }
};
/**
 * @brief Асинхронный таймер
 *
 */
class AsyncTimer : public running::IRunnable
{

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
    TimerInfo createNanoTimer(uint64_t ns, TaskCbPtr &cb, bool is_async = false);
    /**
     * @brief Создание таймера ожидающего ms милисекунд
     *
     * @param ms ожидание в милисекундах
     * @param cb функция выполняющаяся по истечении таймера
     * @param is_async асинхронное выполнение задания
     * @return uint64_t идентификатор таймера или 0 в случае ошибки
     */
    TimerInfo createMilliTimer(uint64_t ms, TaskCbPtr &cb, bool is_async = false);
    /**
     * @brief Создание таймера ожидающего sec секунд
     *
     * @param sec ожидание в секундах
     * @param cb функция выполняющаяся по истечении таймера
     * @param is_async асинхронное выполнение задания
     * @return uint64_t идентификатор таймера или 0 в случае ошибки
     */
    TimerInfo createSecTimer(uint32_t sec, TaskCbPtr &cb, bool is_async = false);
    /**
     * @brief Удаление таймера
     *
     * @param id id таймера
     * @return true Успешное удаление
     * @return false Таймер не найден
     *
     * Тяжелая операция 2*log(n)
     */
    bool deleteTimer(uint64_t id);
    /**
     * @brief Запуск цикла проверки таймеров
     *
     * @param terminate Флаг для остановки цикла проверки
     */
    void run(std::atomic_bool &terminate) override;
    /**
     * @brief Проверить таймеры сейчас (не дожидаясь истечения интервала таймера)
     * @return size_t количество сработавших таймеров
     */
    size_t checkTimersNow();
    /**
     * @brief Получение максимальной задержки в наносекундах
     *
     * @return uint64_t
     * Не потокобезопасен
     */
    uint64_t maxDelay();
    /**
     * @brief Получение максимального количества активных таймеров
     *
     * @return uint64_t
     * Не потокобезопасен
     */
    uint64_t maxSize();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};