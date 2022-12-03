#pragma once

class AsyncTimer
{
public:
    AsyncTimer() = default;
    AsyncTimer(const AsyncTimer &) = delete;
    AsyncTimer(AsyncTimer &&) = delete;
    AsyncTimer &operator=(const AsyncTimer &) = delete;
    AsyncTimer &operator=(AsyncTimer &&) = delete;
    ~AsyncTimer();
};