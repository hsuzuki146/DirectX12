#include "game.h"
#include "../common/timer.h"
#include "../common/d3dx12manager.h"
#include "model/model.h"

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
		const float deltaTime = timer.GetDeltaTime();

		// �X�V�ƕ`��.
		update(deltaTime);
		render();
	}
}

void Game::update( float deltaTime )
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

void Game::updateNone(float deltaTime)
{

}
void Game::updateInitialize(float deltaTime)
{
	if (!D3D_MGR().Create())
	{
		state_ = State::None;
		return;
	}
	Model model;
	model.LoadFromFile("Duck.gltf", "data/" );

	state_ = State::Exec;
}
void Game::updateExec(float deltaTime)
{

}
void Game::updateRelease(float deltaTime)
{
	D3D_MGR().Destroy();
}
