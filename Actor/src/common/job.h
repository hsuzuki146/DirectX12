#pragma once

#include <functional>
#include "common.h"

class JobContainer;

class Job final
{
public:
	Job();
	~Job();

	// 実行処理.
	void Execute(Float32 deltaTime);
	
	// ジョブとして実行するファンクションを設定.
	template<typename T>
	void SetFunction(T* obj, void(T::* func)(Float32));

	// ジョブコンテナの設定.
	void SetContainer(JobContainer* container) { container_ = container; }

private:
	Job(const Job &) = delete;
	Job &operator=(const Job &) = delete;

private:
	std::function<void(Float32)> function_;	// ジョブとして実行する関数オブジェクト.
	
	JobContainer* container_ = { nullptr };	// 登録したコンテナのポインタ.

};

template<typename T>
void Job::SetFunction(T* obj, void(T::* func)(Float32 deltaTime))
{
	function_ = std::bind(std::mem_fn(func), obj, std::placeholders::_1);
}