#pragma once
#include "common.h"

class Timer
{
public:
	Timer();
	~Timer();
	void Initialize();
	void Update();
	float GetDeltaTime() const { return delta_time_; }
private:
	DWORD current_time_ = { 0 };
	float delta_time_ = 0.0f;
};