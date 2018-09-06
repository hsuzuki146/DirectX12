#include "timer.h"

#pragma comment( lib, "winmm.lib")

Timer::Timer()
{
	Initialize();
}
Timer::~Timer()
{
}
void Timer::Initialize()
{
	current_time_ = timeGetTime();
}

void Timer::Update()
{
	const UInt64 time = timeGetTime();
	delta_time_ = static_cast<Float32>(time - current_time_) / 1000.0f;
	current_time_ = time;
}
