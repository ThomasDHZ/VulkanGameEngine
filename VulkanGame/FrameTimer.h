#pragma once
#include <chrono>

class FrameTimer
{
private:
	std::chrono::steady_clock::time_point LastFrame;
	std::chrono::duration<float> FrameTime;
public:
	FrameTimer();
	virtual ~FrameTimer();
	void EndFrameTime();

	float GetFrameTime() { return FrameTime.count(); }
};

