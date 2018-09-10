#pragma once

#include "model/model.h"

class Game
{
public:
	enum class State : int
	{
		None,
		Initialize,
		Exec,
		Release,
	};

public:
	Game();
	~Game();
	void Run();

private:
	void update(float deltaTime);
	void render();

private:
	void updateNone(float deltaTime);
	void updateInitialize(float deltaTime);
	void updateExec(float deltaTime);
	void updateRelease(float deltaTime);
private:
	State state_ = { State::Initialize };

	Model model_;
};