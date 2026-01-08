#include "TimeSystem.h"
#include <glfw3.h>

TimeSystem& timeSystem = TimeSystem::Get();


inline TimeSystem::TimeSystem()
{
    NextFrameTime = glfwGetTime();
    FrameStartTime = 0.0;
    AverageWorkTime = TargetFrameTime;
    UncappedFPS = TargetFPS;
    DeltaTime = 1.0f / TargetFPS;
}

void TimeSystem::StartFrameTimer() 
{
    double currentTime = glfwGetTime();
    double sleepDuration = NextFrameTime - currentTime;
    if (sleepDuration > 0.0) {
        if (sleepDuration > 0.002) {
            std::this_thread::sleep_for(std::chrono::duration<double>(sleepDuration - 0.001));
        }
        currentTime = glfwGetTime();
        while (currentTime < NextFrameTime) {
            // Busy-wait
            currentTime = glfwGetTime();
        }
    }
    FrameStartTime = glfwGetTime();

    // Debug info
    printf("Start Frame - currentTime: %f, NextFrameTime: %f\n", FrameStartTime, NextFrameTime);
}

void TimeSystem::EndFrameTimer() 
{
    double now = glfwGetTime();
    double work_time = now - FrameStartTime;

    RecentWorkTimes.push_back(work_time);
    if (RecentWorkTimes.size() > AverageWindow) RecentWorkTimes.pop_front();

    double sum = 0.0;
    for (double t : RecentWorkTimes) sum += t;

    AverageWorkTime = sum / RecentWorkTimes.size();
    UncappedFPS = (AverageWorkTime > 0.0) ? 1.0 / AverageWorkTime : 0.0;
    NextFrameTime += TargetFrameTime;

    double nowTime = glfwGetTime();
    if (NextFrameTime < nowTime) {
        NextFrameTime = nowTime + TargetFrameTime;
    }

    // Debug info
    printf("End Frame - work_time: %f, Avg WorkTime: %f, FPS: %f\n",
        work_time, AverageWorkTime, UncappedFPS);
}

