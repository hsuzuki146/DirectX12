#pragma once
#include "common.h"

class Timer
{
public:
	Timer();
	~Timer();
	void Initialize();
	void Update();
	Float32 GetDeltaTime() const { return delta_time_; }
private:
	UInt64 current_time_ = { 0 };
	Float32 delta_time_ = 0.0f;
};