#include "FrameTimer.h"
using namespace std::chrono;


FrameTimer::FrameTimer()
{
	FrameTime = std::chrono::duration<float>(0.0f);
	LastFrame = steady_clock::now();
}

FrameTimer::~FrameTimer()
{
}

void FrameTimer::EndFrameTime()
{
	const steady_clock::time_point oldFrameTime = LastFrame;
	LastFrame = steady_clock::now();
	FrameTime = LastFrame - oldFrameTime;
}
