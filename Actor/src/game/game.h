#pragma once

#include "../common/common.h"

class Game
{
public:
	enum class State : UInt32
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
	void update(Float32 deltaTime);
	void render();

private:
	void updateNone(Float32 deltaTime);
	void updateInitialize(Float32 deltaTime);
	void updateExec(Float32 deltaTime);
	void updateRelease(Float32 deltaTime);
private:
	State state_ = { State::Initialize };
};