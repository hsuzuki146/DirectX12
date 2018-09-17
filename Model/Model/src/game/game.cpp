#include "game.h"
#include "../common/timer.h"
#include "../common/d3dx12manager.h"
#include "../common/camera.h"

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
	// �`��O.
	{
		D3D_MGR().Begin();
		D3D_MGR().PreRender();
	}

	// �J�����X�V.
	CAMERA().Update();
	// ���f���`��.
	model_.Draw();

	// �`���.
	{
		D3D_MGR().End();
		D3D_MGR().ExecuteCommand();
		D3D_MGR().WaitFrame();
		D3D_MGR().ResetCommand();
		D3D_MGR().Present();
	}
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

	CAMERA().Initialize();
	CAMERA().Update();

	// ���f���̃��[�h.
	model_.LoadFromFile("Duck.gltf", "data/");
	//model_.LoadFromFile("Box.gltf", "data/");

	state_ = State::Exec;
}
void Game::updateExec(float deltaTime)
{

}
void Game::updateRelease(float deltaTime)
{
	D3D_MGR().Destroy();
}
