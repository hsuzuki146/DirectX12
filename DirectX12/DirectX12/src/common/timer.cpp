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
	const DWORD time = timeGetTime();
	delta_time_ = static_cast<float>(time - current_time_) / 1000.0f;
	current_time_ = time;
}
