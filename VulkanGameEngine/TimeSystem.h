#pragma once
#include "Platform.h"
#include <deque>

class TimeSystem
{
public:
    static TimeSystem& Get();

private:
    TimeSystem();
    ~TimeSystem() = default;
    TimeSystem(const TimeSystem&) = delete;
    TimeSystem& operator=(const TimeSystem&) = delete;
    TimeSystem(TimeSystem&&) = delete;
    TimeSystem& operator=(TimeSystem&&) = delete;

    const double TargetFPS = 60.0;
    const double TargetFrameTime = 1.0 / TargetFPS;
    const uint AverageWindow = 60u;

    std::deque<double> RecentWorkTimes;

    double NextFrameTime;

public:
    double FrameStartTime = 0.0;
    double AverageWorkTime = TargetFrameTime;
    double UncappedFPS = TargetFPS;
    float DeltaTime = 1.0f / TargetFPS;

    DLL_EXPORT void StartFrameTimer();
    DLL_EXPORT void EndFrameTimer();
};
extern DLL_EXPORT TimeSystem& timeSystem;
inline TimeSystem& TimeSystem::Get()
{
    static TimeSystem instance;
    return instance;
}
