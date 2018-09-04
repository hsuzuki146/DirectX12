#include "game.h"
#include "../common/timer.h"
#include "../common/d3dx12manager.h"

#include "actor/actor.h"
#include "actor/component/move_component.h"

Game::Game()
{
}

Game::~Game()
{
}

void Game::Run()
{
	Timer timer;

	while (TRUE)
	{
		MSG msg{};
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				break;
			}
		}

		// 経過時間の更新.
		timer.Update();
		const Float32 deltaTime = timer.GetDeltaTime();

		// 更新と描画.
		update(deltaTime);
		render();
	}
}

void Game::update(Float32 deltaTime )
{
	switch (state_)
	{
		case State::None:		updateNone(deltaTime);			break;
		case State::Initialize:	updateInitialize(deltaTime);	break;
		case State::Exec:		updateExec(deltaTime);			break;
		case State::Release:	updateRelease(deltaTime);		break;
	}
}

void Game::render()
{
	if (state_ != State::Exec)
	{
		return;
	}
	// 描画.
	D3D_MGR().Render();
}

void Game::updateNone(Float32 deltaTime)
{

}
void Game::updateInitialize(Float32 deltaTime)
{
	if (!D3D_MGR().Create())
	{
		state_ = State::None;
		return;
	}
	state_ = State::Exec;
}
void Game::updateExec(Float32 deltaTime)
{

}
void Game::updateRelease(Float32 deltaTime)
{
	D3D_MGR().Destroy();
}
