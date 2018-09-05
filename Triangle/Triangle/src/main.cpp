#include "common/common.h"
#include "application.h"

int main()
{
	SetupParam::Param param;
	param.hInst_ = GetModuleHandle(nullptr);
	param.windowSize_ = SIZE{ 1280,720 };
	param.windowName_ = _T("Triangle");
	SetupParam::GetInstance().SetParam(param);

	Application app;
	return app.Run();
}

