#include "game.h"
#include "../common/timer.h"
#include "../common/d3dx12manager.h"
#include "model/camera.h"

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
		const float deltaTime = timer.GetDeltaTime();

		// 更新と描画.
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
	// 描画前.
	{
		D3D_MGR().Begin();
		D3D_MGR().PreRender();
	}

	// カメラ更新.
	CAMERA().Update();
	// モデル描画.
	model_.Draw();

	// 描画後.
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

	// モデルのロード.
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
