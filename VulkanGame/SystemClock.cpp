#include "SystemClock.h"

using namespace std::chrono;

SystemClock::SystemClock()
{
	TimeSinceOpen = steady_clock::now();
}

SystemClock::~SystemClock()
{
}

float SystemClock::SystemUpTime()
{
	const steady_clock::time_point startTime = TimeSinceOpen;
	const steady_clock::time_point now = steady_clock::now();
	const duration<float> systemTime = now - TimeSinceOpen;
	return systemTime.count();
}

