#include "game.h"
#include "../common/timer.h"
#include "../common/d3dx12manager.h"

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

		// �o�ߎ��Ԃ̍X�V.
		timer.Update();
		const Float32 deltaTime = timer.GetDeltaTime();

		// �X�V�ƕ`��.
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
	// �`��.
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
