#pragma once
#include "chrono"

class SystemClock
{
private: 
	std::chrono::steady_clock::time_point TimeSinceOpen;
public:
	SystemClock();
	virtual ~SystemClock();
	float SystemUpTime();
};

