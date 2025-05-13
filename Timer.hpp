#pragma once

#include <chrono>

using Clock = std::chrono::high_resolution_clock;

// タイマークラスの追加
class Timer {
private:
    Clock::time_point start_time_;
    std::chrono::milliseconds time_limit_;
    bool has_time_limit_;

public:
    Timer(int time_limit_ms = -1) : 
        start_time_(Clock::now()),
        time_limit_(time_limit_ms),
        has_time_limit_(time_limit_ms > 0) {}

    void restart() {
        start_time_ = Clock::now();
    }

    bool isTimeUp() const {
        if (!has_time_limit_) return false;
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - start_time_);
        return elapsed >= time_limit_;
    }

    int elapsedMs() const {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            Clock::now() - start_time_);
        return static_cast<int>(elapsed.count());
    }
};
