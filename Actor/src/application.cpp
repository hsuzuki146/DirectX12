#include "application.h"
#include "common/window.h"
#include "game/game.h"

Int32 Application::Run()
{
	Window::GetInstance().Create();

	Game game;
	game.Run();

	Window::GetInstance().Destroy();
	
	return 0;
}