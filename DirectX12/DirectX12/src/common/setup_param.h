#pragma once
#include "common.h"

class SetupParam : public Singleton <SetupParam>
{
public:
	struct Param
	{
		HINSTANCE hInst_;
		LPCTSTR windowName_;
		SIZE windowSize_;
	};
public:
	void SetParam(const Param& param) { param_ = param; }
	const Param& GetParam() const { return param_; }

private:
	Param param_;
};
